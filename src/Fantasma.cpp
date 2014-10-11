/*
 * Fantasma.cpp
 *
 *  Created on: Oct 4, 2014
 *      Author: juan
 */


#ifdef FANTASMA

#include<iostream>
#include <stdio.h>

#include "Semaforos/Semaforo.h"
#include "Memoria_Compartida/MemoriaCompartida.h"
#include "Locks/LockWrite.hpp"

#include <stdlib.h>
#include <errno.h>
#include "string.h"

#include "logger/Logger.hpp"
#include "utils/Utils.hpp"


 /*
  * Kid fantasma para cerrar la calesita y que no quede bloqueada por semaforos
  *
  */

using namespace std;

//todo Control de errores!!



int main ( int argc, char** argv){

	//Abro el logger
	Logger* logger = new Logger();
	Info* info = new Info(getpid(), "Fantasma");

	logger->log("Entré al parque", info);

	//para la memoria compartida
	MemoriaCompartida<int> continua;
	continua.crear("/etc",55, PERMISOS_USER_RDWR);


	Semaforo semCalGira("/etc", 22);
	semCalGira.crear();
	Semaforo semCalLug("/etc", 23);
	semCalLug.crear();

	//prepara lock de kidsInPark
	LockFile* lockWContinua = new LockWrite("archLockCont");

	lockWContinua->tomarLock();
	continua.escribir(1);
	lockWContinua->liberarLock();

	//intenta subir a la calecita


	logger->log("Intento subir a la calesita", info);

	semCalLug.p(-1);


	logger->log("Subí a la calesita", info);

	//espera que la calesita gire
	semCalGira.p(-1);

	logger->log("Me bajé de la calesita", info);


	//libero memoria compartida
	continua.liberar();

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

