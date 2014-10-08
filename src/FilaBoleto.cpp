/*
 * Puerta.cpp
 *
 *  Created on: Sep 30, 2014
 *      Author: juan
 */


#ifdef FILA1

#include "Seniales/SignalHandler.h"
#include "Seniales/SIGUSR1_Handler.h"
#include <memory.h>
#include "Pipes_y_Fifos/FifoEscritura.h"
#include "Pipes_y_Fifos/Pipe.h"

#include "logger/Logger.hpp"
#include "utils/Utils.hpp"
#include "Constantes.h"

 /*
  * Controla las colas de chicos con pipes
  */

using namespace std;


//todo Control de errores!!


int main ( int argc, char** argv){

	//Abro el logger
	Logger* logger = Logger::getLogger();
	logger->setOutput("LOG.log");
	logger->init();
	Info* info = new Info(getpid(), "FilaBoleto");

	logger->log("Arranca la fila de venta de boletos",info);

	//pongo el manejador de la señal
	SIGUSR1_Handler sigusr1_handler;
	SignalHandler :: getInstance()->registrarHandler ( SIGUSR1,&sigusr1_handler );

	//recibe pipes
	int fdReadKid,fdWriteKid,fdReadPuerta,fdWritePuerta;

	fdReadKid = toInt(argv[1]);
	fdWriteKid = toInt(argv[2]);
	fdReadPuerta = toInt(argv[3]);
	fdWritePuerta = toInt(argv[4]);

	Pipe pipe1(fdReadKid,fdWriteKid);
	pipe1.setearModo(Pipe::LECTURA);

	Pipe pipe2(fdReadPuerta,fdWritePuerta);
	pipe2.setearModo(Pipe::ESCRITURA);

	FifoEscritura fifoRecaudador("FifoRecaudador");
	fifoRecaudador.abrir();

	bool seguir = true;

	while (seguir){

		int pidKid;
		pipe1.leer(&pidKid, sizeof(int));

		if (pidKid != -1){//-1 llega para que muera la puerta

			string ruta = "Cola" + toString(pidKid);
			//todo ver que pasaba si se llena un pipe

			//le dice al chico que pase
			FifoEscritura fifoAKid(ruta);
			fifoAKid.abrir();

			logger->log("Le da boleto al chico: " + toString(pidKid),info);
			fifoAKid.escribir( &VALOR_PASAR, sizeof(int) );

			fifoAKid.cerrar();
			fifoAKid.eliminar();

			//le avisa al recaudador que pago un chico
			int pago = 1;
			logger->log("Le dice al recaudador que paso el chico: " + toString(pidKid),info);
			fifoRecaudador.escribir( &pago, sizeof(int));

			//le mete el niño a la otra fila
			pipe2.escribir(&pidKid, sizeof(int));
		}else{
			seguir=false;
			//Le pasa el -1 a la otra fila para que muera
			int msj = -1;
			pipe2.escribir(&msj, sizeof(int));
		}
	}

	//para que muera el recaudador
	int pago = 2;
	fifoRecaudador.escribir( &pago, sizeof(int));

	fifoRecaudador.cerrar();
	fifoRecaudador.eliminar();

	pipe1.cerrar();
	pipe2.cerrar();

	SignalHandler :: destruir ();

	logger->log("Se cierra la fila de venta de boletos",info);

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


