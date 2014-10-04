//============================================================================
// Name        : CalecitaVpropia.cpp
// Author      : Juan Fuld
// Version     :
// Copyright   : None
// Description : Hello World in C++, Ansi-style
//============================================================================

#ifdef CALESITA

#include<iostream>
#include <stdio.h>

#include "Memoria_Compartida/MemoriaCompartida.h"
#include "Locks/LockWrite.hpp"
#include "Locks/LockRead.hpp"
#include <sys/sem.h>
#include <sys/wait.h>

#include <unistd.h>
#include <stdlib.h>
#include <errno.h>

#include "logger/Logger.hpp"
#include "utils/Utils.hpp"

using namespace std;

/*
 * La calecita
 * */


static const int CANTPARAM = -1;

//todo Control de errores!!


//todo que no loggee si muero == true!
int main(int argc, char** argv) {

	//PARAMETROS
	//tiempoVuelta: Cuanto dura una vuelta de la calesita
	//cantMaxLugares: Cuantos lugares tiene la calesita en total
	//lugaresLibres: cuantos lugares abre para la proxima vuelta
	int tiempoVuelta, cantMaxLugares, lugaresLibres;

	//todo extraer a funcion
	if ( argc != 4 ){
		cout << "Cantidad de parametros incorrectos, especifique duracion y cantidad de lugares" << endl;
		return CANTPARAM;
	}

	stringstream ss;

	ss.str("");
	ss.clear();
	ss << argv[1];
	ss >> lugaresLibres;

	ss.str("");
	ss.clear();
	ss << argv[2];
	ss >> cantMaxLugares;

	ss.str("");
	ss.clear();
	ss << argv[3];
	ss >> tiempoVuelta;

	//Fin recepcion de parametros

	//obtiene la memoria compartida
	MemoriaCompartida<int> kidsInPark;
	kidsInPark.crear("arch",33);

	MemoriaCompartida<int> continua;
	continua.crear("arch",55);

	//obtiene los semaforos para sincronizarse
	int key = ftok("arch",22);
	int key2 = ftok("arch",23);
	int key4 = ftok("arch",24);

	int semId = semget( key, 1, IPC_CREAT|0666);
	int semId2 = semget( key2, 1, IPC_CREAT|0666);
	int semId4 = semget( key4, 1, IPC_CREAT|0666); //para control de cola de entrada a la calesita

	struct sembuf operacion[1];

	//prepara locks de kidsInPark
	LockFile* lockW = new LockWrite("archLockKids");
	LockFile* lockR = new LockRead("archLockKids");
	LockFile* lockR2 = new LockRead("archLockCont");

	int seguir = 0;

	while (seguir == 0){
		operacion[0].sem_num = 0;
		operacion[0].sem_op = 0; //arranca cuando no hay mas lugares
		operacion[0].sem_flg = 0;

		semop(semId2, operacion, 1 );

		/************DEBUG***********************/
		ushort arreglo2[2];

		union semnum {
			int val;
			struct semid_ds* buf;
			ushort* array;
		};

		semnum init2;
		init2.array=arreglo2;
		semctl(semId2,0,GETPID,init2);
		int pid = init2.val;
		cout << semctl(semId2,0,GETALL,init2)<<endl;
		cout<< "Semaf calesita antes de la vuelta: " << init2.array[0] << "Tocado por: " << pid << endl;

		/************************************************************/

		if (seguir == 0){
			//no duerme con el fantasma
			sleep(tiempoVuelta);
		}

		//avisa a los chicos que termino la vuelta
		operacion[0].sem_num = 0;
		cout<<"Lugares libres: "<< lugaresLibres<<endl;
		operacion[0].sem_op = lugaresLibres; //A tantos como giraron (que son los que deberian estar esperando esta señal)
		operacion[0].sem_flg = 0;

		semop(semId, operacion, 1 );


		//Despues de girar baja la cantidad de pendejos esperando
		lockW->tomarLock();

		int chicosRestantes = kidsInPark.leer() - lugaresLibres;
		kidsInPark.escribir(chicosRestantes);

		lockW->liberarLock();

		//primero se fija que todos los niños hayan bajado
		operacion[0].sem_num = 0;
		operacion[0].sem_op = 0;
		operacion[0].sem_flg = 0;

		semop(semId, operacion, 1 );

		//segundo, se bajan los pendejos -> abre lugares para los que queden y avisa a puerta que los deje pasar

		lockR->tomarLock();
		int kidsWaiting = kidsInPark.leer();
		lockR->liberarLock();

		//segun cuantos chicos estan esperando es la cantidad de lugares que abre
		if (kidsWaiting >= cantMaxLugares){
			lugaresLibres = cantMaxLugares;
		}else{
			lugaresLibres = kidsWaiting;
		}

		//siempre tiene por lo menos 1 lugar libre
		if (lugaresLibres <= 0 )
			{lugaresLibres = 1;};

		operacion[0].sem_num = 0;
		operacion[0].sem_op = lugaresLibres;
		operacion[0].sem_flg = 0;

		int res = semop(semId2, operacion, 1 );
		res = semop(semId4, operacion, 1 );

		lockR2->tomarLock();
		seguir = continua.leer();
		lockR2->liberarLock();

	}

	cout<<"Salgo del whiule y termino: " <<endl;

	//libero memoria compartida y demas cosas
	kidsInPark.liberar();
	continua.liberar();

	delete lockW;
	delete lockR;
	delete lockR2;

	return 0;
}

#endif
