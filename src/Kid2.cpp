/*
 * Kid.cpp
 *
 *  Created on: Sep 26, 2014
 *      Author: juan
 */

#ifdef KID

#include<iostream>
#include <stdio.h>


#include <sys/sem.h>

#include "Memoria_Compartida/MemoriaCompartida.h"
#include "Pipes_y_Fifos/Pipe.h"
#include "Pipes_y_Fifos/FifoLectura.h"
#include "Locks/LockWrite.hpp"
#include "Locks/LockRead.hpp"

#include <stdlib.h>
#include <errno.h>
#include "string.h"

#include "logger/Logger.hpp"
#include "utils/Utils.hpp"


 /*
  * Kid
  *
  * Aumenta KidsInPark al comenzar con shMem y locks
  *	Se mete en la cola para comprar boleto con pipe
  *	Corre para subir a la calesita con semaforo
  *
  */

using namespace std;

//todo Control de errores!!



int main ( int argc, char** argv){

	//Abro el logger
	Logger* logger = obtenerLogger();
	Info* info = new Info(getpid(), "Kid");

	logger->log("Entré al parque", info);
	//recibe pipes
	int fdRdPuerta1,fdWrPuerta1;

	fdRdPuerta1 = toInt(argv[1]);
	fdWrPuerta1  = toInt(argv[2]);

	Pipe pipePuerta1(fdRdPuerta1,fdWrPuerta1);

	pipePuerta1.setearModo(Pipe::ESCRITURA);

	//para la memoria compartida
	MemoriaCompartida<int> kidsInPark;
	kidsInPark.crear("/etc",33, PERMISOS_USER_RDWR); //todo permisos!


//todo considerar permisos, hacerlos restrictivos
//todo encapsular semaforos (esperar a que den el codigo en la practica)
	//para hacer operaciones del semaforo
	struct sembuf operations[1];

	int key = ftok("/etc",22);
	int key2 = ftok("/etc",23);
	int key3 = ftok("/etc",24);

	int semId = semget( key, 1, IPC_CREAT|0666);
	int semId2 = semget( key2, 1, IPC_CREAT|0666);
	int semId3 = semget( key3, 1, IPC_CREAT|0666); //mutex entrada y salida

	//prepara lock de kidsInPark
	LockFile* lockW = new LockWrite("archLockKids");

	//aca consideramos que entro a la calesita

	//para que entre "de a uno" uso semaforo binario para seccion critica
	operations[0].sem_num = 0;
	operations[0].sem_op = -1;
	operations[0].sem_flg = 0;
	semop(semId3, operations, 1);

	logger->log("Entré a la calesita", info);

	//como entro aumenta cantidad de chicos presentes...
	lockW->tomarLock();

	kidsInPark.escribir(kidsInPark.leer()+1);
	logger->log("Aumento en uno la cantidad de chicos en el parque", info);

	lockW->liberarLock();


	operations[0].sem_num = 0;
	operations[0].sem_op = 1;
	operations[0].sem_flg = 0;
	semop(semId3, operations, 1);

	//se mete en la cola de boletos

	logger->log("Me encolo para sacar boleto", info);
//Meterse es pasarle a la puerta por donde le tiene que escribir para desbloquearlo
	int pid = getpid();
	pipePuerta1.escribir( &pid, sizeof(int) );

//Espera que la puerta le escriba "pasa" por el fifo corresponidente
	int permisoPasar = 0;

	string ruta = "Cola" + toString(getpid());
	FifoLectura fila(ruta);
	fila.abrir();
	fila.leer(&permisoPasar, sizeof(int) );

//todo sacar TODOS los numeros magicos, pasar a constantes.h

	logger->log("Obtuve boleto " + toString(permisoPasar), info);

//Espera que la segunda puerta le escriba "pasa"
	FifoLectura fila2(ruta + "C");
	fila2.abrir();
	int leer = fila2.leer(&permisoPasar, sizeof(int) );

	cout << leer << endl;

	logger->log("Pase la cola para subir a la calesita " + toString(permisoPasar), info);

	//intenta subir a la calecita

	operations[0].sem_num = 0;
	operations[0].sem_op = -1;
	operations[0].sem_flg = 0;

	semop(semId2, operations, 1);

	logger->log("Me subí a la calesita", info);

	//espera que la calesita gire
	operations[0].sem_num = 0;
	operations[0].sem_op = -1;
	operations[0].sem_flg = 0;

	semop(semId, operations, 1);

	logger->log("Me bajé de la calesita", info);

	//libero memoria compartida
	kidsInPark.liberar();

  	//cierro pipe a las puerta y fifo de la cola
	pipePuerta1.cerrar();

	fila.cerrar();
	fila.eliminar();

	delete lockW;

	//para que salga "de a uno" uso semaforo binario para seccion critica, uso el mismo de la entrada
	// por fiaca de no crear otro mas, a lo sumo no hay chicos entrando al mismo tiempo que otros salen
	operations[0].sem_num = 0;
	operations[0].sem_op = -1;
	operations[0].sem_flg = 0;
	semop(semId3, operations, 1);

	logger->log("Me fui del parque", info);
	//todo preguntar si habria que hacer algo particular aca...

	operations[0].sem_num = 0;
	operations[0].sem_op = 1;
	operations[0].sem_flg = 0;
	semop(semId3, operations, 1);

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


