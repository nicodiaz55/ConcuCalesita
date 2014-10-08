/* Puerta2.cpp
 *
 *  Created on: Oct 3, 2014
 *      Author: juan
 */


#ifdef FILA2

#include "Seniales/SignalHandler.h"
#include "Seniales/SIGUSR1_Handler.h"
#include <memory.h>
#include "Pipes_y_Fifos/FifoEscritura.h"
#include "Pipes_y_Fifos/Pipe.h"
#include <sys/sem.h>

#include "logger/Logger.hpp"
#include "utils/Utils.hpp"
#include "Constantes.h"

 /*
  * Controla la cola de chicos despues de que tengan boleto
  */

using namespace std;


//todo Control de errores!!


int main ( int argc, char** argv){

	//Abro el logger
	Logger* logger = Logger::getLogger();
	logger->setOutput("LOG.log");
	logger->init();
	Info* info = new Info(getpid(), "Fila Calesita");

	logger->log("Arranca la fila de la calesita",info);

	//pongo el manejador de la señal
	SIGUSR1_Handler sigusr1_handler;
	SignalHandler :: getInstance()->registrarHandler ( SIGUSR1,&sigusr1_handler );

	//recibe pipes
	int fdReadPuerta,fdWritePuerta;

	fdReadPuerta = toInt(argv[1]);
	fdWritePuerta  = toInt(argv[2]);

	Pipe pipe(fdReadPuerta,fdWritePuerta);
	pipe.setearModo(Pipe::LECTURA);

	//agarra semaforo
	int key4 = ftok("/etc",25);
	int semId4 = semget( key4, 1, IPC_CREAT|0666); //para control de cola de entrada a la calesita

	struct sembuf operacion[1];

	bool seguir = true;

	while (seguir){
		int pidKid;
		pipe.leer(&pidKid, sizeof(int));

		if (pidKid != -1) {
			string ruta = "Cola" + toString(pidKid);
			//todo ver que pasaba si se llena un pipe

			//le dice al chico que pase. No hay problema con usar el mismo fifo en ambas filas.

			//para que no deje pasar a mas de los que pueden subir le "pregunta" a la calesita cunatos quiere
			operacion[0].sem_num = 1;
			operacion[0].sem_op = -1;
			operacion[0].sem_flg = 0;

			semop(semId4, operacion, 1 );

			FifoEscritura fifoAKid(ruta+ "C");
			fifoAKid.abrir();

			logger->log("Pasa el chico: " + toString(pidKid),info);
			fifoAKid.escribir( &VALOR_PASAR2, sizeof(int) );

			fifoAKid.cerrar();
			fifoAKid.eliminar();
		}else{
			seguir = false;
		}

	}

	pipe.cerrar();

	SignalHandler :: destruir ();

	logger->log("Se cierra la fila de la calesita",info);
	//cierro el logger
	if (logger != NULL) {
		delete logger;
		logger = NULL;
	}
	if (info != NULL) {
		delete info;
		info = NULL;
	}

return 0;

}

#endif
