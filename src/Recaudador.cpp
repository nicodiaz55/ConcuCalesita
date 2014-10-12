/*
 * Recaudador.cpp
 *
 *  Created on: Sep 30, 2014
 *      Author: juan
 */

#ifdef RECAUDADOR


#include "Pipes_y_Fifos/FifoLectura.h"
#include "Locks/LockWrite.hpp"
#include "Locks/LockRead.hpp"
#include <signal.h>

#include "logger/Logger.hpp"
#include "utils/Utils.hpp"
#include "Caja.h"

 /*
  * Registra en la caja el cobro de boletos
  */

using namespace std;


//todo Control de errores!!


//todo cambiar a pasado por parametro
static const int precio = 2;

int main ( int argc, char** argv){
	//Abro el logger
	Logger* logger = new Logger();
	Info* info = new Info(getpid(), "Recaudador");

	logger->log("Entré a trabajar", info);

	FifoLectura fifo("FifoRecaudador");
	int res = fifo.abrir();
	if ( controlErrores2(res, logger, info) == MUERTE_POR_ERROR) { kill(getppid(),SIGINT);}


	//pide memoria comp. para caja
	Caja caja;
	res = caja.init();
	if ( controlErrores1(res, logger, info) == MUERTE_POR_ERROR) { kill(getppid(),SIGINT);}

	//prepara lock de caja recaudacion
	LockFile* lockW = new LockWrite("archLockCaja");

	while (true){

		int avisoPago;
		fifo.leer(&avisoPago , sizeof(int));

		if (avisoPago == 2) { break; }
		lockW->tomarLock();
		if ( controlErrores1(res, logger, info) == MUERTE_POR_ERROR) { kill(getppid(),SIGINT);}

		res = caja.aumentarRecaudacion(precio);
		if ( controlErrores1(res, logger, info) == MUERTE_POR_ERROR) { kill(getppid(),SIGINT);}

		logger->log("Coloqué $" + toString(precio) + "en la caja", info);

		lockW->liberarLock();
		if ( controlErrores1(res, logger, info) == MUERTE_POR_ERROR) { kill(getppid(),SIGINT);}

	}

	//todo IMP!!!
	//todo Que no pase de aca hasta que el administrador tenga tomada la memoria compartida
	//todo (semaforo wait aca y signal en admin)

	//para que muera el administrador
	lockW->tomarLock();
	if ( controlErrores1(res, logger, info) == MUERTE_POR_ERROR) { kill(getppid(),SIGINT);}

	caja.setearEstado(false);

	lockW->liberarLock();
	if ( controlErrores1(res, logger, info) == MUERTE_POR_ERROR) { kill(getppid(),SIGINT);}

	//libero memoria compartida
	res = caja.terminar();
	if ( controlErrores1(res, logger, info) == MUERTE_POR_ERROR) { kill(getppid(),SIGINT);}

 	delete lockW;

	fifo.cerrar();
	fifo.eliminar();

	logger->log("Terminé mi trabajo", info);

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


