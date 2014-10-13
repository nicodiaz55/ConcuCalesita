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
#include "Semaforos/Semaforo.h"
#include <signal.h>

#include "logger/Logger.hpp"
#include "utils/Utils.hpp"
#include "Caja.h"

 /*
  * Registra en la caja el cobro de boletos
  */

using namespace std;

int main ( int argc, char** argv){
	//Abro el logger
	Logger* logger = new Logger();
	Info* info = new Info(getpid(), "Recaudador");

	logger->log("Entré a trabajar", info);

	int precio = toInt(argv[1]);

	FifoLectura fifo("FifoRecaudador");
	int res = fifo.abrir();
	if ( controlErrores2(res, logger, info) == MUERTE_POR_ERROR ) { kill(getppid(),SIGINT);}

	//Semaforo para sincronizar con el administrador (caso especial 0 chicos)
	Semaforo semAdminRec("/etc", SEM_ADMIN_REC);
	res = semAdminRec.crear();
	if ( controlErrores1(res, logger, info) == MUERTE_POR_ERROR ) { kill(getppid(),SIGINT);}

	//pide memoria comp. para caja
	Caja caja;
	res = caja.init();
	if ( controlErrores1(res, logger, info) == MUERTE_POR_ERROR ) { kill(getppid(),SIGINT);}

	//prepara lock de caja recaudacion
	LockFile* lockW = new LockWrite("archLockCaja");

	while (true){

		int avisoPago;
		fifo.leer(&avisoPago , sizeof(int));

		if (avisoPago == 2) { break; }
		res = lockW->tomarLock();
		if ( controlErrores2(res, logger, info) == MUERTE_POR_ERROR ) { kill(getppid(),SIGINT);}

		res = caja.aumentarRecaudacion(precio);
		controlErrores1(res, logger, info);

		logger->log("Coloqué $" + toString(precio) + " en la caja", info);

		res = lockW->liberarLock();
		if ( controlErrores2(res, logger, info) == MUERTE_POR_ERROR ) { kill(getppid(),SIGINT);}

	}

	//Para que no pase de aca hasta que el administrador tenga tomada la memoria compartida
	semAdminRec.p(-1);

	//para que muera el administrador
	res = lockW->tomarLock();
	if ( controlErrores2(res, logger, info) == MUERTE_POR_ERROR ) { kill(getppid(),SIGINT);}

	caja.setearEstado(false);

	res = lockW->liberarLock();
	if ( controlErrores2(res, logger, info) == MUERTE_POR_ERROR ) { kill(getppid(),SIGINT);}

	//libero memoria compartida
	res = caja.terminar();
	if ( controlErrores1(res, logger, info) == MUERTE_POR_ERROR ) { kill(getppid(),SIGINT);}

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


