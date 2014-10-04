/*
 * LanzadorProcesos.cpp
 *
 *  Created on: Sep 26, 2014
 *      Author: juan
 */

#ifdef LANZADOR

#include<iostream>
#include <stdio.h>

#include <sys/sem.h>
#include "Memoria_Compartida/MemoriaCompartida.h"
#include "Pipes_y_Fifos/Pipe.h"
#include <sys/wait.h>
#include <signal.h>
#include <memory.h>

#include <stdlib.h>

#include "logger/Logger.hpp"
#include "utils/Utils.hpp"


 /*
  * Lanza chicos (Procesos Kid) con fork y exec
  * Lanza calesita, administrador y recaudador
  *
  * Inicializa semaforos y shMem antes de que haya otros procesos para prevenir competencias.
  */

//todo 1) Sacar arch y arch2 pasarlo a /dev y /etc o lo que sea
//todo 2) Limpiar includes de cosas innecesarias

using namespace std;

static const int CANTPARAM = -1;
//todo Control de errores!!

int main ( int argc, char** argv){

	//Abro el logger
	Logger* logger = Logger::getLogger();
	logger->setOutput("test.log");
	logger->init();
	Info* info = new Info(getpid(), "Lanzador");

	logger->log("Park is open", info);

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
	int key1 = ftok("arch",22);
	int key2 = ftok("arch",23);
	int key3 = ftok("arch",24);
	int key4 = ftok("arch",24);

	int semId = semget( key1, 1, IPC_CREAT|0666); //calecita girando
	int semId2 = semget( key2, 1, IPC_CREAT|0666); //lugares calecita
	int semId3 = semget( key3, 1, IPC_CREAT|0666); //mutex entrada y salida
	int semId4 = semget( key4, 1, IPC_CREAT|0666); //para control de cola de entrada a la calesita

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

	semctl (semId2, 0, SETVAL, init );
	semctl (semId4, 0, SETVAL, init );//empiezan igual

	init.val = 1; //al ppio alguien puede entrar (o salir)

	semctl (semId3, 0, SETVAL, init );

	//inicializa memoria compartida.
//todo sacar a funcion

	MemoriaCompartida<int> kidsInPark;
	kidsInPark.crear("arch",33); //todo cambiar permisos (que los tome por param)
	//Memoria compartida de cantidad de chicos en parque
	kidsInPark.escribir(0);

	MemoriaCompartida<int> caja;
	caja.crear("arch",44); //todo cambiar permisos (que los tome por param)
	//Memoria compartida para la caja
	caja.escribir(0);

	MemoriaCompartida<int> continua;
	continua.crear("arch",55); //todo cambiar permisos (que los tome por param)
	//Memoria compartida para la caja
	continua.escribir(0);

//LANZAR RECAUDADOR Y ADMINISTRADOR

	pid_t pidRec = fork();

	if (pidRec == 0){
		execl("Recaudador", "Recaudador", (char*)0);
	}

	pid_t pidAdmin = fork();

	if (pidAdmin == 0){
		execl("Administrador", "Administrador",(char*)0);
	}
//desattachea de la caja una vez que la tienen Recaud. y Admin.
	caja.liberar();

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

	continua.liberar();

	//LANZAR NIÑOS Y PUERTAS

	//crea los pipes que van de los niños a las puertas y entre puertas y lanza puertas

	Pipe pipeEntrePuertas;
	int fdRdPuerta2 = pipeEntrePuertas.getFdLectura();
	int fdWrPuerta2 = pipeEntrePuertas.getFdEscritura(); //para escribirle a la puerta 2

	//todo REVISAR RECEPCION PARAMETROS EN TODOS LADOS!!
	pid_t pidPuerta2 = fork();
	//todo ALERTA NEGRADA: aca la puerta 2 tiene abierto el pipe de la 1, cambiar o a otro proceso o a usar fifos
	if (pidPuerta2 == 0){
		execl("FilaCalesita", "Puerta2", toString(fdRdPuerta2).c_str(), toString(fdWrPuerta2).c_str(), (char*)0);
	}

	//pipe para que los ninios se encolen para boleto
	Pipe pipeAKids;
	int fdRdPuerta1 = pipeAKids.getFdLectura();
	int fdWrPuerta1 = pipeAKids.getFdEscritura(); //para escribirle a la puerta 1

	pid_t pidPuerta1 = fork();

	if (pidPuerta1 == 0){
		execl("FilaBoleto", "Fila1", toString(fdRdPuerta1).c_str(), toString(fdWrPuerta1).c_str(), toString(fdRdPuerta2).c_str(), toString(fdWrPuerta2).c_str(), (char*)0);
	}

	//lanza niños
	for (int i = 0; i<cantNinios ; i++){

		pid_t pid = fork();

		if (pid == 0){
			execl("Kid", "Kid", toString(fdRdPuerta1).c_str(), toString(fdWrPuerta1).c_str(), (char*)0);
		}
	}

	//se desattachea esta shared mem. despues de crear los hijos, asi siempre hay alguien usandola.
	kidsInPark.liberar ();

	//espero que terminen todos los hijos
	int status;
	for (int i = 0; i<cantNinios ; i++){
		wait(&status);
	}


	//Señales a puertas y calesita para que mueran

	kill(pidPuerta1, SIGUSR1);
	kill(pidPuerta2, SIGUSR1);

	//espera a las puertas
	//todo cambiar por waitpid
	int aux = -1;
	write(fdWrPuerta1,&aux,sizeof(int));
	write(fdWrPuerta2,&aux,sizeof(int));

	//cierro ambos descriptores porque el lanzador no usa estos pipes
	//los tengo abiertos hasta aca porque igual no hay forks que los dupliquen en otros procesos
	//y porque los necesito para desbloquear la puerta para que termine seguro
	pipeAKids.cerrar();
	pipeEntrePuertas.cerrar();

	struct sembuf operacion[1];

	wait(&status); //todo waitpid

	//para destrabar la puerta 2
	operacion[0].sem_num = 1;
	operacion[0].sem_op = 1;
	operacion[0].sem_flg = 0;

	semop(semId4, operacion, 1 );

	wait(&status);

//espera a la calesita
//puede que la calesita quede bloqueada esperando a un ultimo niño-> mando un fantasma

//hace que un "chico fantasma" de la ultima vuelta
	pid_t pidFantasma = fork();

	if (pidFantasma == 0){

		execl("Fantasma","Fantasma", (char*) 0);

	}

	//todo cambiar por waitpid de calesita
	wait(&status);

	//espera al recaudador y admin todo waitpid
	wait(&status);
	wait(&status);

	//mato el semaforo
	semctl(semId,0,IPC_RMID);
	semctl(semId2,0,IPC_RMID);
	semctl(semId3,0,IPC_RMID);
	semctl(semId4,0,IPC_RMID);

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
