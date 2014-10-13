/*
 * Fantasma.cpp
 *
 *  Created on: Oct 4, 2014
 *      Author: juan
 */


#ifdef FANTASMA

#include "Semaforos/Semaforo.h"
#include "Memoria_Compartida/MemoriaCompartida.h"
#include "Locks/LockWrite.hpp"
#include <signal.h>

#include "logger/Logger.hpp"
#include "utils/Utils.hpp"


 /*
  * Kid fantasma para cerrar la calesita y que no quede bloqueada por semaforos
  *
  */

using namespace std;

int main ( int argc, char** argv){

	//Abro el logger
	Logger* logger = new Logger();
	Info* info = new Info(getpid(), "Fantasma");

	logger->log("Entré al parque", info);

	//para la memoria compartida
	MemoriaCompartida<int> continua;
	int res = continua.crear("/etc",55, PERMISOS_USER_RDWR);
	if ( controlErrores1(res, logger, info) == MUERTE_POR_ERROR) { kill(getppid(),SIGINT);}

	//semaforos
	Semaforo semCalGira("/etc", 22);
	res = semCalGira.crear();
	if ( controlErrores1(res, logger, info) == MUERTE_POR_ERROR) { kill(getppid(),SIGINT);}

	Semaforo semCalLug("/etc", 23);
	res = semCalLug.crear();
	if ( controlErrores1(res, logger, info) == MUERTE_POR_ERROR) { kill(getppid(),SIGINT);}


	//prepara lock de kidsInPark
	LockFile* lockWContinua = new LockWrite("archLockCont");

	res = lockWContinua->tomarLock();
	if ( controlErrores1(res, logger, info) == MUERTE_POR_ERROR) { kill(getppid(),SIGINT);}

	continua.escribir(1);

	res = lockWContinua->liberarLock();
	if ( controlErrores1(res, logger, info) == MUERTE_POR_ERROR) { kill(getppid(),SIGINT);}

	//intenta subir a la calesita


	logger->log("Intento subir a la calesita", info);

	res = semCalLug.p(-1);
	if ( controlErrores1(res, logger, info) == MUERTE_POR_ERROR) { kill(getppid(),SIGINT);}

	logger->log("Subí a la calesita", info);

	//espera que la calesita gire
	res = semCalGira.p(-1);
	if ( controlErrores1(res, logger, info) == MUERTE_POR_ERROR) { kill(getppid(),SIGINT);}

	logger->log("Me bajé de la calesita", info);


	//libero memoria compartida
	res = continua.liberar();
	if ( controlErrores1(res, logger, info) == MUERTE_POR_ERROR) { kill(getppid(),SIGINT);}

	logger->log("Como mis deseos infantiles fueron satisfecho, me retiro de este mundo", info);

	delete lockWContinua;

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

