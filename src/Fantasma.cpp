/*
 * Fantasma.cpp
 *
 *  Created on: Oct 4, 2014
 *      Author: juan
 */


#ifdef FANTASMA

#include<iostream>
#include <stdio.h>

#include <sys/sem.h>

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
	Logger* logger = Logger::getLogger();
	logger->setOutput("LOG.log");
	logger->init();
	Info* info = new Info(getpid(), "Fantasma");

	logger->log("Entré al parque", info);

	//para la memoria compartida
	MemoriaCompartida<int> continua;
	continua.crear("/etc",55);

	//para hacer operaciones del semaforo
	struct sembuf operations[1];

	int key = ftok("/etc",22);
	int key2 = ftok("/etc",23);
	int key3 = ftok("/etc",24);

	int semId = semget( key, 1, IPC_CREAT|0666);
	int semId2 = semget( key2, 1, IPC_CREAT|0666);

	//prepara lock de kidsInPark
	LockFile* lockW = new LockWrite("archLockCont");

	lockW->tomarLock();
	continua.escribir(1);
	lockW->liberarLock();

	//intenta subir a la calecita

	operations[0].sem_num = 0;
	operations[0].sem_op = -1;
	operations[0].sem_flg = 0;

	logger->log("Intneto subir a la calesita", info);

	semop(semId2, operations, 1);


	logger->log("Subí a la calesita", info);

	//espera que la calesita gire
	operations[0].sem_num = 0;
	operations[0].sem_op = -1;
	operations[0].sem_flg = 0;

	semop(semId, operations, 1);

	logger->log("Me bajé de la calesita", info);


	//libero memoria compartida
	continua.liberar();


	delete lockW;

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

