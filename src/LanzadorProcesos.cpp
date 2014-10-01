/*
 * LanzadorProcesos.cpp
 *
 *  Created on: Sep 26, 2014
 *      Author: juan
 */


#ifdef LANZADOR

#include<iostream>
#include <stdio.h>

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <signal.h>
#include <memory.h>

#include <unistd.h>
#include <stdlib.h>

#include "logger/Logger.hpp"
#include "utils/Utils.hpp"
 /*
  * Lanza chicos (Procesos Kid) con fork y exec
  * Lanza calesita, administrador y recaudador
  *
  * Inicializa semaforos y shMem antes de que haya otros procesos para prevenir competencias.
  */

using namespace std;

static const int CANTPARAM = -1;
//todo Control de errores!!

int main ( int argc, char** argv){

	int cantNinios = 0;
	int lugaresCalesita = 0;
	int tiempoVuelta = 0;

//LECTURA DE PARAMETROS
		//todo extraer a funcion
		if ( argc < 4 ){
			cout << "Cantidad de parametros incorrectos, especifique numero de niños y cantidad de lugares" << endl;
			return CANTPARAM;
		}

		int option;
		stringstream ss;

		while ((option = getopt(argc, argv, "l:n:t:")) != -1){
			switch (option) {

				case 't':
					//todo extraer conversion a funcion
					ss.str("");
					ss.clear();
					ss << optarg;
					ss >> tiempoVuelta;
					break;

				case 'l':
					//todo extraer conversion a funcion
					ss.str("");
					ss.clear();
					ss << optarg;
					ss >> lugaresCalesita;
					break;

				case 'n':
					//todo ver el de arriba
					ss.str("");
					ss.clear();
					ss << optarg;
					ss >> cantNinios;
					break;

				default:
					cout << "Opcion invalida" << endl;
					break;

			}
		}

//INICIALIZAR IPC mechanisms
	int key = ftok("arch",22);

	int semId = semget( key, 2, IPC_CREAT|0666); //2 semaforos, calecita girando y lugares calecita

	union semnum {
		int val;
		struct semid_ds* buf;
		ushort* array;
	};

	semnum init;
	init.val = 0; //sem cola de calecita girando esta en cero

	semctl (semId, 0, SETVAL, init );

	//si los chicos no alcanzan a llenarla solo espera a que se sienten todos para arrancar
	if (cantNinios < lugaresCalesita){
		init.val = cantNinios;
	}else{
		init.val = lugaresCalesita;
	}

	int lugaresCalesitaUsados = init.val;

	semctl (semId, 1, SETVAL, init );

	//inicializa memoria compartida.
	int keyShM = ftok("arch",33);
	int shMId = shmget( keyShM, sizeof(int), IPC_CREAT|0666); //Memoria compartida para aumentar cantidad de chicos
	void* shMpN = shmat(shMId, NULL, 0);

	int* kidsInPark = static_cast<int*> (shMpN);

	(*kidsInPark) = 0;

	keyShM = ftok("arch",44);
	shMId = shmget( keyShM, sizeof(int), IPC_CREAT|0666); //Memoria compartida para aumentar cantidad de chicos
	void* shMp = shmat(shMId,NULL,0);

	int* caja = static_cast<int*> (shMp);

	(*caja) = 0;

//LANZAR NIÑOS Y PUERTAS

	int fd[2];
//crea los pipes que van de los niños a las puertas
	pipe(fd);
	int fdRdPuerta1 = fd [0];
	int fdWrPuerta1 = fd[1]; //para escribirle a la puerta 1

	pipe(fd);
	int fdRdPuerta2 = fd [0];
	int fdWrPuerta2 = fd[1]; //para escribirle a la puerta 2
	//todo niños cerrar el de lectura, puerta cerrar el de escritura

	int fdRdNinio = 0;
	int fdWrNinio = 0;
	for (int i = 0; i<cantNinios ; i++){

		pipe(fd); //pipes de las puertas a los niños
		fdRdNinio = fd[0];
		fdWrNinio = fd [1];
		//todo niño debe cerrar el fd de escritura

		pid_t pid = fork();

		if (pid == 0){
			cout<< "Recibe Read: " << fdRdNinio << " Write: "<<fdWrPuerta1 << " Y " << fdWrPuerta2 << endl;
			execl("Kid", "Kid", toString(fdRdNinio).c_str() , toString(fdWrPuerta1).c_str(), toString(fdWrPuerta2).c_str(), toString(fdWrNinio).c_str(),(char*)0);
		}
	}

//se desattachea despues de crear los hijos, asi siempre hay alguien usandola
	shmdt (shMpN);

//pipe de la puerta al recaudador
	pipe(fd);
	int fdWrRecaudador, fdRdRecaudador;
	fdWrRecaudador = fd[1];
	fdRdRecaudador = fd[0];

//lanza las puertas
	pid_t pidPuerta1 = fork();

	if (pidPuerta1 == 0){
		cout<< "Recibe Read: "<< fdRdPuerta2 <<endl;
		execl("Puerta", "Puerta2", toString(fdRdPuerta2).c_str(), toString(fdWrRecaudador).c_str(),(char*)0);
	}

	pid_t pidPuerta2 = fork();

	if (pidPuerta2 == 0){
		cout<< "Recibe Read: "<< fdRdPuerta1 <<" Write: "<<fd[1]<<endl;
		execl("Puerta", "Puerta1", toString(fdRdPuerta1).c_str(), toString(fdWrRecaudador).c_str(),(char*)0);
	}

	pid_t pidRec = fork();

	if (pidRec == 0){
		cout<< "Recibe Read: "<< fdRdPuerta1 <<" Write: "<<fd[1]<<endl;
		execl("Recaudador", "Recaudador", toString(fdRdRecaudador).c_str(),(char*)0);
	}

	pid_t pidAdmin = fork();

	if (pidAdmin == 0){
		execl("Administrador", "Administrador",(char*)0);
	}

	shmdt(shMp);

//LANZAR CALESITA

	//todo conversiones auxiliares modularizar
	//uso el stringstream de antes
	stringstream ss1, ss2, ss3;
	ss1.str("");ss2.str("");ss3.str("");
	ss1.clear();ss2.clear();ss3.clear();

//	ss1 << "-l";
	ss1 << lugaresCalesitaUsados;

//	ss2 << "-c";
	ss2 << lugaresCalesita;

//	ss3 << "-t";
	ss3 << tiempoVuelta;

	pid_t pidCal;
	pidCal = fork();

	if (pidCal == 0) {
		execl("Calesita", "Calesita", ss1.str().c_str(), ss2.str().c_str(), ss3.str().c_str(), (char*)0);
	}

	//espero que terminen todos los hijos
	int status;
	for (int i = 0; i<cantNinios ; i++){

		wait(&status);
	}


	//Señales a puertas y calesita para que mueran
cout<< "les digo que mueran" << endl;


	kill(pidPuerta1, SIGUSR1);
	kill(pidPuerta2, SIGUSR1);
	kill(pidCal, SIGUSR1);

	//espera a las puertas
	//todo cambiar por waitpid
	int aux = -1;
	write(fdWrPuerta1,&aux,sizeof(int));
	write(fdWrPuerta2,&aux,sizeof(int));

	wait(&status);
	wait(&status);

	//espera a la calesita
	//todo cambiar por waitpid
	struct sembuf operacion[1];

	operacion[0].sem_num = 1;
	operacion[0].sem_op = -1;
	operacion[0].sem_flg = SEM_UNDO;

	semop(semId, operacion, 1 );

	wait(&status);

	//espera al recaudador y admin
	wait(&status);
	wait(&status);

	//mato el semaforo
	semctl(semId,0,IPC_RMID);

return 0;

}

#endif


