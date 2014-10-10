/*
 * Administrador.cpp
 *
 *  Created on: Oct 1, 2014
 *      Author: juan
 */

#ifdef ADMIN

#include "Memoria_Compartida/MemoriaCompartida.h"
#include "Locks/LockRead.hpp"

#include "logger/Logger.hpp"
#include "utils/Utils.hpp"


 /*
  * Mira la caja de boletos e imprime por pantalla o al log o whatever
  */

using namespace std;

//todo Control de errores!!



int main ( int argc, char** argv){

	//Abro el logger
	Logger* logger = obtenerLogger();
	Info* info = new Info(getpid(), "Administrador");

	logger->log("Entré al trabajo", info);
	//pide memoria comp. para caja
	MemoriaCompartida<int> caja;
	caja.crear("/etc",44, PERMISOS_USER_RDWR);

	//prepara lock de caja recaudacion
	LockFile* lockR = new LockRead("archLockCaja");

	while (true){

		lockR->tomarLock();

		if (caja.leer() >= 0){
			logger->log("En la caja hay: " + toString(caja.leer()), info);
		}else{
			break;
		}

		lockR->liberarLock();

		//este sleep esta para que no llene el log solo con sus lecturas
		sleep(1);

	}

	//libero memoria compartida
	caja.liberar();

	delete lockR;

	//cierro el logger
	if (logger != NULL) {
		delete logger;
		logger = NULL;
	}
	if (info != NULL) {
		delete info;
		info = NULL;
	}

	logger->log("Me voy del trabajo", info);

return 0;

}

#endif

