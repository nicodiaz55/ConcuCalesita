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


	//para la memoria compartida
	MemoriaCompartida<int> continua;
	continua.crear("arch",55);

	//para hacer operaciones del semaforo
	struct sembuf operations[1];

	int key = ftok("arch",22);
	int key2 = ftok("arch",23);
	int key3 = ftok("arch",24);

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

	cout<< "Quiero calesita Fantasma! "<< getpid() << endl;

	semop(semId2, operations, 1);


	cout<< "Subí a la calesita Fantasma! "<< getpid() << endl;

	//espera que la calesita gire
	operations[0].sem_num = 0;
	operations[0].sem_op = -1;
	operations[0].sem_flg = 0;

	semop(semId, operations, 1);

	cout<< "Me bajé Fantasma! "<< getpid() << endl;


	//libero memoria compartida
	continua.liberar();

  	//cierro pipe a las puerta y fifo de la cola
	delete lockW;

  	return 0;

}

#endif

