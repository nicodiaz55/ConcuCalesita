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

#include <sys/types.h>

#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/wait.h>
# include <fcntl.h>

#include <unistd.h>
#include <stdlib.h>
#include <errno.h>

#include "logger/Logger.hpp"
#include "utils/Utils.hpp"

using namespace std;

/*
 * La calecita
 * */

bool numeroAtt(int Id){

	shmid_ds estado;
	shmctl ( Id,IPC_STAT,&estado );
	return estado.shm_nattch;

}

static const int CANTPARAM = -1;

//todo Control de errores!!
bool seguir = true;

void manejaSig(int sig){
	if(sig == SIGUSR1){
		cout<< "MUERO" << endl;
		seguir = false;
	}
}


int main(int argc, char** argv) {

	//pongo el manejador de la señal
	struct sigaction sa;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	sa.sa_handler = manejaSig;
	sigaction(SIGUSR1,&sa,0);

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
	int keyShM = ftok("arch",33);

	int shMId = shmget( keyShM, sizeof(int), IPC_CREAT|0666);
	void* shMp = shmat(shMId,NULL,0);

	int* kidsInPark = static_cast<int*> (shMp);
cout<< "Cant niños calesita1:" << (*kidsInPark) << endl;
	//obtiene los semaforos para sincronizarse
	int key = ftok("arch",22);
	int semId = semget( key, 2, IPC_CREAT|0666);

	struct sembuf operacion[1];

	union semnum {
			int val;
			struct semid_ds* buf;
			ushort* array;
	};

	//prepara lock de kidsInPark
	struct flock fl;

	fl . l_type = F_WRLCK ;
	fl . l_whence = SEEK_SET ;
	fl . l_start = 0;
	fl . l_len = 0;
	fl . l_pid = getpid () ;
	int fd = open ( "arch" , O_CREAT|O_WRONLY ,0777);

	while (seguir){
		operacion[0].sem_num = 1;
		operacion[0].sem_op = 0; //arranca cuando no hay mas lugares
		operacion[0].sem_flg = SEM_UNDO;

		semop(semId, operacion, 1 );

		/************DEBUG***********************/
		ushort arreglo2[2];

		semnum init2;
		init2.array=arreglo2;
		cout << semctl(semId,1,GETALL,init2)<<endl;
		cout<< "Semaf calesita al arrancar lugares son: " << init2.array[1] << endl;


		/************************************************************/


		sleep(tiempoVuelta);

		//avisa a los chicos que termino la vuelta
		operacion[0].sem_num = 0;
		operacion[0].sem_op = lugaresLibres; //A tantos como giraron (que son los que deberian estar esperando esta señal)
		operacion[0].sem_flg = SEM_UNDO;

		semop(semId, operacion, 1 );


		//Despues de girar baja la cantidad de pendejos esperando
		fl . l_type = F_WRLCK ;
		fcntl ( fd , F_SETLKW ,&fl );

		(*kidsInPark) -= lugaresLibres;
cout<< "Cant niños calesita 2:" << (*kidsInPark) << endl;
		fl . l_type = F_UNLCK ;
		fcntl ( fd , F_SETLK ,&fl );


		/************DEBUG***********************/
		cout << semctl(semId,1,GETALL,init2)<<endl;
		cout<< "Semaf calesita por aca lugares son: " << init2.array[1] << endl;


		/************************************************************/

		//se bajan los pendejos -> abre lugares para los que queden

		//todo lock lectura
		int kidsWaiting = (*kidsInPark);
cout<< "chicos esperando segun calesita: "<<kidsWaiting << endl;
		//segun cuantos chicos estan esperando es la cantidad de lugares que abre
		if (kidsWaiting >= cantMaxLugares){
			lugaresLibres = cantMaxLugares;
		}else{
			lugaresLibres = kidsWaiting;
		}

		//por lo menos tiene 1 lugar libre
		if (lugaresLibres == 0) lugaresLibres = 1;

	/************DEBUG***********************/
	cout << semctl(semId,1,GETALL,init2)<<endl;
	cout<< "Semaf calesita por aca lugares son: " << init2.array[1] << endl;


	/************************************************************/

		operacion[0].sem_num = 1;
		operacion[0].sem_op = lugaresLibres;
		operacion[0].sem_flg = SEM_UNDO;

		semop(semId, operacion, 1 );
		cout << "lugaresLibres" << lugaresLibres << endl;
		/************DEBUG***********************/
		ushort arreglo[2];

		semnum init;
		init.array=arreglo;
		cout << semctl(semId,1,GETALL,init)<<endl;
		cout<< "Semaforo calesita vale1: " << init.array[1] << endl;


		/************************************************************/
	}
	//libero memoria compartida
	int res = shmdt (shMp);

	if (res == -1)
		perror("error");

	cout<<"Salgo del whiule y termino: " <<numeroAtt(shMId)<<endl;

	if (numeroAtt(shMId) == 0){
		res = shmctl(shMId, IPC_RMID, NULL);
		cout<<"Chau mem shared"<<endl;
		if (res == -1)
			perror("error");
	}

	//libero archivo del lock
  	close (fd) ;

	return 0;
}

#endif
