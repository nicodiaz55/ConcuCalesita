/*
 * Kid.cpp
 *
 *  Created on: Sep 26, 2014
 *      Author: juan
 */

#ifdef KID

#include<iostream>
#include <stdio.h>


#include "Semaforos/Semaforo.h"
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
	Logger* logger = new Logger();
	Info* info = new Info(getpid(), "Kid\t");

	logger->log("Voy hacia el parque :D", info);
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

	Semaforo semCalGira("/etc", 22); // para bajarse
	semCalGira.crear();
	Semaforo semMutexEntrada("/etc", 24); // puerta de entrada/salida
	semMutexEntrada.crear();
	Semaforo semCalLug("/etc", 23); // para subirse
	semCalLug.crear();

	//prepara lock de kidsInPark
	LockFile* lockW = new LockWrite("archLockKids");

	//aca consideramos que entro a la calesita

	//para que entre "de a uno" uso semaforo binario para seccion critica
	semMutexEntrada.p(-1);

	logger->log("Entré al parque", info);

	//como entro aumenta cantidad de chicos presentes...
	lockW->tomarLock();

	kidsInPark.escribir(kidsInPark.leer()+1);
	logger->log("Aumento en uno la cantidad de chicos en el parque", info);

	lockW->liberarLock();

	semMutexEntrada.v(1);


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

	logger->log("Obtuve mi boleto " + toString(permisoPasar), info);

//Espera que la segunda puerta le escriba "pasa"
	FifoLectura fila2(ruta + "C");
	fila2.abrir();
	int leer = fila2.leer(&permisoPasar, sizeof(int) );

	cout << leer << endl;

	logger->log("Pasé la cola para subir a la calesita " + toString(permisoPasar), info);

	//intenta subir a la calecita
	semCalLug.p(-1);

	logger->log("Me subí a la calesita :D", info);

	//espera que la calesita gire
	semCalGira.p(-1);

	logger->log("Me bajé de la calesita :(", info);

	//libero memoria compartida
	kidsInPark.liberar();

  	//cierro pipe a las puerta y fifo de la cola
	pipePuerta1.cerrar();

	fila.cerrar();
	fila.eliminar();

	delete lockW;

	//para que salga "de a uno" uso semaforo binario para seccion critica, uso el mismo de la entrada
	// por fiaca de no crear otro mas, a lo sumo no hay chicos entrando al mismo tiempo que otros salen

	semMutexEntrada.p(-1);

	logger->log("Me fui del parque", info);
	//todo preguntar si habria que hacer algo particular aca...

	semMutexEntrada.v(1);

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


