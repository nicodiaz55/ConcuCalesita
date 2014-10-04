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


 /*
  * Controla la cola de chicos despues de que tengan boleto
  */

using namespace std;


//todo Control de errores!!


int main ( int argc, char** argv){

	//pongo el manejador de la señal
	SIGUSR1_Handler sigusr1_handler;
	SignalHandler :: getInstance()->registrarHandler ( SIGUSR1,&sigusr1_handler );

	//recibe pipes
	int fdReadPuerta,fdWritePuerta;

	stringstream ss;
	ss.str("");
	ss.clear();
	ss << argv[1];
	ss >> fdReadPuerta;

	ss.str("");
	ss.clear();
	ss << argv[2];
	ss >> fdWritePuerta;

	Pipe pipe(fdReadPuerta,fdWritePuerta);
	pipe.setearModo(Pipe::LECTURA);

	//agarra semaforo
	int key4 = ftok("arch",24);
	int semId4 = semget( key4, 1, IPC_CREAT|0666); //para control de cola de entrada a la calesita

	struct sembuf operacion[1];

	while (sigusr1_handler.getGracefulQuit() != 1){
		int fdWr;
		pipe.leer(&fdWr, sizeof(int));

		string ruta = "Cola" + toString(fdWr);
//todo ver que pasaba si se llena un pipe

//le dice al chico que pase. No hay problema con usar el mismo fifo en ambas filas porque tienen
//que escribir en orden por construccion.
//(Ademas de que abren y cierran con lo cual no son el mismo Fifo [me siento Heraclito])
		//para que no deje pasar a mas de los que pueden subir
		operacion[0].sem_num = 1;
		operacion[0].sem_op = -1;
		operacion[0].sem_flg = 0;

		semop(semId4, operacion, 1 );

		FifoEscritura fifoAKid(ruta);

		int pasa = 8;
		fifoAKid.escribir( &pasa, sizeof(int) );

		fifoAKid.cerrar();
		fifoAKid.eliminar();

	}

	pipe.cerrar();

	SignalHandler :: destruir ();

return 0;

}

#endif
