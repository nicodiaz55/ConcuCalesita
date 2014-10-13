/*
 * Administrador.cpp
 *
 *  Created on: Oct 1, 2014
 *      Author: juan
 */

#ifdef ADMIN

#include "Locks/LockRead.hpp"
#include "Semaforos/Semaforo.h"
#include <signal.h>

#include "utils/Random.hpp"
#include "logger/Logger.hpp"
#include "utils/Utils.hpp"
#include "Caja.h"

 /*
  * Mira la caja de boletos e imprime por pantalla o al log o whatever
  */

using namespace std;

//todo Control de errores!!



int main ( int argc, char** argv){

	//Abro el logger
	Logger* logger = new Logger();
	Info* info = new Info(getpid(), "Administrador");

	//Semaforo para sincronizar con el recaudador (caso especial 0 chicos)
	Semaforo semAdminRec("/etc", SEM_ADMIN_REC);
	int res = semAdminRec.crear();
	if ( controlErrores1(res, logger, info) == MUERTE_POR_ERROR ) { kill(getppid(),SIGINT);}

	logger->log("Entré a trabajar", info);
	//pide memoria comp. para caja
	Caja caja;
	res = caja.init();
	if ( controlErrores1(res, logger, info) == MUERTE_POR_ERROR) { kill(getppid(),SIGINT);}

	//ahora puede terminar el recaudador
	semAdminRec.v(1);

	//prepara lock de caja recaudacion
	LockFile* lockR = new LockRead("archLockCaja");

	while (true){

		res = lockR->tomarLock();
		if ( controlErrores2(res, logger, info) == MUERTE_POR_ERROR) { kill(getppid(),SIGINT);}

		logger->log("En la caja hay: $" + toString(caja.obtenerRecaudacion()), info);
		if (caja.obtenerEstado() == false) {
			break;
		}

		res = lockR->liberarLock();
		if ( controlErrores2(res, logger, info) == MUERTE_POR_ERROR) { kill(getppid(),SIGINT);}
		//este sleep esta para que no llene el log solo con sus lecturas
		sleep(uniform(1,2));

	}

	//libero memoria compartida
	res = caja.terminar();
	if ( controlErrores1(res, logger, info) == MUERTE_POR_ERROR) { kill(getppid(),SIGINT);}

	delete lockR;

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

