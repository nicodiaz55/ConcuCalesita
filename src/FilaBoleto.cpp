/*
 * Puerta.cpp
 *
 *  Created on: Sep 30, 2014
 *      Author: juan
 */


#ifdef FILA1

#include "Pipes_y_Fifos/FifoEscritura.h"
#include "Pipes_y_Fifos/Pipe.h"
#include <signal.h>

#include "logger/Logger.hpp"
#include "utils/Utils.hpp"


 /*
  * Controla las colas de chicos con pipes
  */

using namespace std;

int main ( int argc, char** argv){

	//Abro el logger
	Logger* logger = new Logger();
	Info* info = new Info(getpid(), "FilaBoleto");

	logger->log("Arranca la fila de venta de boletos",info);

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

	FifoEscritura fifoRecaudador(FIFORECAUDADOR);
	int res = fifoRecaudador.abrir();
	if ( controlErrores2(res, logger, info) == MUERTE_POR_ERROR) { kill(getppid(),SIGINT);}

	bool seguir = true;

	while (seguir){

		int pidKid;
		res = pipe1.leer(&pidKid, sizeof(int));
		if (res != sizeof(int)){
			logger->log("Atencion: se leyeron del pipe solo: " + toString(res), info);
		}

		if (pidKid != CERRAR_FILA){//-1 llega para que muera la puerta

			string ruta = PREFIJO_FIFO_FILA_KIDS + toString(pidKid);

			//le dice al chico que pase
			FifoEscritura fifoAKid(ruta);
			res = fifoAKid.abrir();
			if ( controlErrores2(res, logger, info) == MUERTE_POR_ERROR) { kill(getppid(),SIGINT);}

			logger->log("Le da boleto al chico: " + toString(pidKid),info);

			res = fifoAKid.escribir( &VALOR_PASAR, sizeof(int) );
			if (res != sizeof(int)){
				logger->log("Atencion: se escribieron al pipe solo: " + toString(res), info);
			}

			fifoAKid.cerrar();
			fifoAKid.eliminar();

			//le avisa al recaudador que pago un chico
			int pago = AVISO_DE_PAGO;
			logger->log("Le dice al recaudador que pasó el chico: " + toString(pidKid),info);
			res = fifoRecaudador.escribir( &pago, sizeof(int));
			if (res != sizeof(int)){
				logger->log("Atencion: se escribieron al pipe solo: " + toString(res), info);
			}

			//le mete el niño a la otra fila
			res = pipe2.escribir(&pidKid, sizeof(int));
			if (res != sizeof(int)){
				logger->log("Atencion: se escribieron al pipe solo: " + toString(res), info);
			}
		}else{
			seguir=false;
			//Le pasa el -1 a la otra fila para que muera
			int msj = CERRAR_FILA;
			res = pipe2.escribir(&msj, sizeof(int));
			if (res != sizeof(int)){
				logger->log("Atencion: se escribieron al pipe solo: " + toString(res), info);
			}
		}
	}

	//para que muera el recaudador
	int pago = FIN_PAGOS;
	res = fifoRecaudador.escribir( &pago, sizeof(int));
	if (res != sizeof(int)){
		logger->log("Atencion: se escribieron al pipe solo: " + toString(res), info);
	}

	fifoRecaudador.cerrar();
	fifoRecaudador.eliminar();

	pipe1.cerrar();
	pipe2.cerrar();

	logger->log("Se cierra la fila de venta de boletos",info);

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


