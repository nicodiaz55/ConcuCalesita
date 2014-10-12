/* Puerta2.cpp
 *
 *  Created on: Oct 3, 2014
 *      Author: juan
 */


#ifdef FILA2

#include "Pipes_y_Fifos/FifoEscritura.h"
#include "Pipes_y_Fifos/Pipe.h"
#include "Semaforos/Semaforo.h"
#include <signal.h>

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
	Logger* logger = new Logger();
	Info* info = new Info(getpid(), "Fila Calesita");

	logger->log("Arranca la fila de la calesita",info);

	//recibe pipes
	int fdReadPuerta,fdWritePuerta;

	fdReadPuerta = toInt(argv[1]);
	fdWritePuerta  = toInt(argv[2]);

	Pipe pipeEntrePuertas(fdReadPuerta,fdWritePuerta);
	pipeEntrePuertas.setearModo(Pipe::LECTURA);

	//agarra semaforo
	Semaforo semColaCal("/etc", 25);
	int res = semColaCal.crear();
	if ( controlErrores1(res, logger, info) == MUERTE_POR_ERROR) { kill(getppid(),SIGINT);}

	bool seguir = true;

	while (seguir){
		int pidKid;
		pipeEntrePuertas.leer(&pidKid, sizeof(int));


		if (pidKid != -1) {
			string ruta = "Cola" + toString(pidKid);
			//todo ver que pasaba si se llena un pipe

			//le dice al chico que pase. No hay problema con usar el mismo fifo en ambas filas.

			//para que no deje pasar a mas de los que pueden subir le "pregunta" a la calesita cunatos quiere
			res = semColaCal.p(-1);
			if ( controlErrores1(res, logger, info) == MUERTE_POR_ERROR) { kill(getppid(),SIGINT);}

			FifoEscritura fifoAKid(ruta + "C");
			res = fifoAKid.abrir();
			if ( controlErrores2(res, logger, info) == MUERTE_POR_ERROR) { kill(getppid(),SIGINT);}

			logger->log("Pasa el chico: " + toString(pidKid),info);

			fifoAKid.escribir( &VALOR_PASAR2, sizeof(int) );

			fifoAKid.cerrar();
			fifoAKid.eliminar();
		}else{
			seguir = false;
		}

	}

	pipeEntrePuertas.cerrar();

	logger->log("Se cierra la fila de la calesita",info);

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
