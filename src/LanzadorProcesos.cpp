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

#include "Constantes.h"
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

	//tengo que meter esta parte antes del logger o nadie puede entrar, incluyendo al lanzador
	int key5 = ftok("/etc",100);

	int semId5 = semget( key5, 1, IPC_CREAT|0666); //para loggear

	union semnum {
		int val;
		struct semid_ds* buf;
		ushort* array;
	};
	semnum init;

	init.val = 1; //al ppio alguien puede loggear
	semctl (semId5, 0, SETVAL, init );

	//Abro el logger
	Logger* logger = Logger::getLogger();
	logger->setOutput("LOG.log");
	logger->init();
	Info* info = new Info(getpid(), "Lanzador");

	logger->log("Empieza la simulación", info);

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
					tiempoVuelta = toInt(optarg);
					logger->log("El tiempo de vuelta es de: " + toString(tiempoVuelta) + " segundos", info);
					break;

				case 'l':
					lugaresCalesita = toInt(optarg);
					logger->log("La cantidad de lugares es: " + toString(lugaresCalesita), info);
					break;

				case 'n':
					cantNinios = toInt(optarg);
					logger->log("La cantidad de niños en el barrio es: " + toString(cantNinios), info);
					break;

				default:
					logger->log("Se ingresó una opción inválida.", info);
					break;

			}
		}

//INICIALIZAR IPC mechanisms
	int key1 = ftok("/etc",22);
	int key2 = ftok("/etc",23);
	int key3 = ftok("/etc",24);
	int key4 = ftok("/etc",25);

	int semId = semget( key1, 1, IPC_CREAT|0666); //calecita girando
	int semId2 = semget( key2, 1, IPC_CREAT|0666); //lugares calecita
	int semId3 = semget( key3, 1, IPC_CREAT|0666); //mutex entrada y salida
	int semId4 = semget( key4, 1, IPC_CREAT|0666); //para control de cola de entrada a la calesita

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
	kidsInPark.crear("/etc",33); //todo cambiar permisos (que los tome por param)
	//Memoria compartida de cantidad de chicos en parque
	kidsInPark.escribir(0);

	MemoriaCompartida<int> caja;
	caja.crear("/etc",44); //todo cambiar permisos (que los tome por param)
	//Memoria compartida para la caja
	caja.escribir(0);

	MemoriaCompartida<int> continua;
	continua.crear("/etc",55); //todo cambiar permisos (que los tome por param)
	//Memoria compartida para la caja
	continua.escribir(0);

//LANZAR RECAUDADOR Y ADMINISTRADOR

	pid_t pidRec = fork();

	if (pidRec == 0){
		execl("Recaudador", "Recaudador", (char*)0);
	}

	logger->log("Se lanzó el recaudador", info);

	pid_t pidAdmin = fork();

	if (pidAdmin == 0){
		execl("Administrador", "Administrador",(char*)0);
	}

	logger->log("Se lanzó el administrador", info);
//desattachea de la caja una vez que la tienen Recaud. y Admin.
	caja.liberar();

//LANZAR CALESITA

	string arg1 = toString(lugaresCalesitaUsados);
	string arg2 = toString(lugaresCalesita);
	string arg3 = toString(tiempoVuelta);

	pid_t pidCal;
	pidCal = fork();

	if (pidCal == 0) {
		execl("Calesita", "Calesita", arg1.c_str(), arg2.c_str(), arg3.c_str(), (char*)0);
	}

	logger->log("Se lanzó la calesita", info);

	//libero para que ya la maneje solo la calesita (y mas tarde el fantasma)
	continua.liberar();

	//LANZAR NIÑOS Y PUERTAS

	//crea los pipes que van de los niños a las puertas y entre puertas y lanza puertas

	Pipe pipeEntrePuertas;
	int fdRdPuerta2 = pipeEntrePuertas.getFdLectura();
	int fdWrPuerta2 = pipeEntrePuertas.getFdEscritura(); //para escribirle a la puerta 2

	//todo REVISAR RECEPCION PARAMETROS EN TODOS LADOS!!
	pid_t pidPuerta2 = fork();

	if (pidPuerta2 == 0){
		execl("FilaCalesita", "Puerta2", toString(fdRdPuerta2).c_str(), toString(fdWrPuerta2).c_str(), (char*)0);
	}

	logger->log("Se lanzó la fila para subir a la calesita", info);

	//pipe para que los ninios se encolen para boleto
	Pipe pipeAKids;
	int fdRdPuerta1 = pipeAKids.getFdLectura();
	int fdWrPuerta1 = pipeAKids.getFdEscritura(); //para escribirle a la puerta 1

	pid_t pidPuerta1 = fork();

	if (pidPuerta1 == 0){
		execl("FilaBoleto", "Fila1", toString(fdRdPuerta1).c_str(), toString(fdWrPuerta1).c_str(), toString(fdRdPuerta2).c_str(), toString(fdWrPuerta2).c_str(), (char*)0);
	}

	logger->log("Se lanzó la fila para comprar boleto", info);

	pipeEntrePuertas.cerrar();

	//lanza niños
	for (int i = 0; i<cantNinios ; i++){

		pid_t pid = fork();

		if (pid == 0){
			execl("Kid", "Kid", toString(fdRdPuerta1).c_str(), toString(fdWrPuerta1).c_str(), (char*)0);
		}

		logger->log("Se lanzó un niño", info);
	}

	//se desattachea esta shared mem. despues de crear los hijos, asi siempre hay alguien usandola.
	kidsInPark.liberar ();

	//espero que terminen todos los hijos
	int status;
	for (int i = 0; i<cantNinios ; i++){
		wait(&status);
	}


	//Le dice a las puertas que mueran

	//espera a las puertas
	//todo cambiar por waitpid
	int aux = -1;
	write(fdWrPuerta1,&aux,sizeof(int));

	//cierro descriptor
	//lo tengo abierto hasta aca porque igual no hay forks que los dupliquen en otros procesos
	//y porque lo necesito para desbloquear la puerta para que termine seguro
	pipeAKids.cerrar();

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
	logger->log("Se lanzó el niño fantasma", info);

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
	semctl(semId5,0,IPC_RMID);

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


