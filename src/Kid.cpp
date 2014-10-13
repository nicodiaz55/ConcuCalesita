/*
 * Kid.cpp
 *
 *  Created on: Sep 26, 2014
 *      Author: juan
 */

#ifdef KID

#include "Semaforos/Semaforo.h"
#include "Memoria_Compartida/MemoriaCompartida.h"
#include "Memoria_Compartida/VectorMemoCompartida.h"
#include "Pipes_y_Fifos/Pipe.h"
#include "Pipes_y_Fifos/FifoLectura.h"
#include "Locks/LockWrite.hpp"
#include "Locks/LockRead.hpp"
#include <signal.h>

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
	//recibe pipes y cantidad de lugares
	int fdRdPuerta1,fdWrPuerta1;

	fdRdPuerta1 = toInt(argv[1]);
	fdWrPuerta1  = toInt(argv[2]);
	int cantlugares = toInt(argv[3]);

	Pipe pipePuerta1(fdRdPuerta1,fdWrPuerta1);

	pipePuerta1.setearModo(Pipe::ESCRITURA);

	//para la memoria compartida
	MemoriaCompartida<int> kidsInPark;
	int res = kidsInPark.crear("/etc",33, PERMISOS_USER_RDWR);
	if ( controlErrores1(res, logger, info) == MUERTE_POR_ERROR ) { raise (SIGINT);}

	VectorMemoCompartida<bool> lugares;

	res = lugares.crear("/etc",INICIO_CLAVES_LUGARES,PERMISOS_USER_RDWR,cantlugares);
	if ( controlErrores1(res, logger, info) == MUERTE_POR_ERROR ) { raise (SIGINT);}

//todo considerar permisos, hacerlos restrictivos

	Semaforo semCalGira("/etc", 22); // para bajarse
	res = semCalGira.crear();
	if ( controlErrores1(res, logger, info) == MUERTE_POR_ERROR ) { raise (SIGINT);}

	Semaforo semMutexEntrada("/etc", 24); // puerta de entrada/salida
	res = semMutexEntrada.crear();
	if ( controlErrores1(res, logger, info) == MUERTE_POR_ERROR ) { raise (SIGINT);}

	Semaforo semCalLug("/etc", 23); // para subirse
	res = semCalLug.crear();
	if ( controlErrores1(res, logger, info) == MUERTE_POR_ERROR ) { raise (SIGINT);}

	//prepara lock de kidsInPark
	LockFile* lockKids = new LockWrite("archLockKids");

	//prepara el lock de los lugares
	LockFile* lockSpots = new LockWrite(ARCH_LOCK_LUGARES);

	//aca consideramos que entro a la calesita

	//para que entre "de a uno" uso semaforo binario para seccion critica
	res = semMutexEntrada.p(-1);
	if ( controlErrores1(res, logger, info) == MUERTE_POR_ERROR ) { raise (SIGINT);}

	logger->log("Entré al parque", info);

	//como entro aumenta cantidad de chicos presentes...
	lockKids->tomarLock();
	if ( controlErrores1(res, logger, info) == MUERTE_POR_ERROR ) { raise (SIGINT);}

	kidsInPark.escribir(kidsInPark.leer()+1);
	logger->log("Aumento en uno la cantidad de chicos en el parque", info);

	lockKids->liberarLock();
	if ( controlErrores1(res, logger, info) == MUERTE_POR_ERROR ) { raise (SIGINT);}

	res = semMutexEntrada.v(1);
	if ( controlErrores1(res, logger, info) == MUERTE_POR_ERROR ) { raise (SIGINT);}

	//se mete en la cola de boletos

	logger->log("Me encolo para sacar boleto", info);
//Meterse es pasarle a la puerta por donde le tiene que escribir para desbloquearlo
	int pid = getpid();
	pipePuerta1.escribir( &pid, sizeof(int) );

//Espera que la puerta le escriba "pasa" por el fifo corresponidente
	int permisoPasar = 0;

	string ruta = "Cola" + toString(getpid());
	FifoLectura fila(ruta);
	res = fila.abrir();
	if ( controlErrores2(res, logger, info) == MUERTE_POR_ERROR) { kill(getppid(),SIGINT);}

	fila.leer(&permisoPasar, sizeof(int) );
//todo sacar TODOS los numeros magicos, pasar a constantes.h

	logger->log("Obtuve mi boleto ", info);

//Espera que la segunda puerta le escriba "pasa"
	FifoLectura fila2(ruta + "C");
	res = fila2.abrir();
	if ( controlErrores2(res, logger, info) == MUERTE_POR_ERROR) { kill(getppid(),SIGINT);}
	res = fila2.leer(&permisoPasar, sizeof(int) );

	logger->log("Pasé la cola para subir a la calesita ", info);

	//intenta subir a la calecita
	res = semCalLug.p(-1);
	if ( controlErrores1(res, logger, info) == MUERTE_POR_ERROR) { kill(getppid(),SIGINT);}

	//Cuando logra subirse intenta ir al lugar que quiere
	bool libre;

	for (int i = 0 ; i < cantlugares; i++){
		lockSpots->tomarLock();
		if ( controlErrores1(res, logger, info) == MUERTE_POR_ERROR ) { raise (SIGINT);}

		libre = lugares.leer(i);
		if (libre == LUGAR_LIBRE){
			logger->log("Me senté en el lugar: " + toString(i), info);
			lugares.escribir(LUGAR_OCUPADO,i);
			break;
		}else{
			string msj = "Quize el lugar: " + toString(i) + " ,pero estaba ocupado.";
			logger->log(msj, info);
		}

		lockSpots->liberarLock();
		if ( controlErrores1(res, logger, info) == MUERTE_POR_ERROR ) { raise (SIGINT);}
	}



	logger->log("Me subí a la calesita :D", info);

	//espera que la calesita gire
	res = semCalGira.p(-1);
	if ( controlErrores1(res, logger, info) == MUERTE_POR_ERROR) { kill(getppid(),SIGINT);}

	logger->log("Me bajé de la calesita :(", info);

	//libero memoria compartida
	res = kidsInPark.liberar();
	if ( controlErrores1(res, logger, info) == MUERTE_POR_ERROR) { kill(getppid(),SIGINT);}

	res = lugares.liberar();
	if ( controlErrores1(res, logger, info) == MUERTE_POR_ERROR) { kill(getppid(),SIGINT);}
  	//cierro pipe a las puerta y fifo de la cola
	pipePuerta1.cerrar();

	fila.cerrar();
	fila.eliminar();

	delete lockKids;
	delete lockSpots;

	//para que salga "de a uno" uso semaforo binario para seccion critica, uso el mismo de la entrada
	// por fiaca de no crear otro mas, a lo sumo no hay chicos entrando al mismo tiempo que otros salen

	res = semMutexEntrada.p(-1);
	if ( controlErrores1(res, logger, info) == MUERTE_POR_ERROR) { kill(getppid(),SIGINT);}

	logger->log("Me fui del parque", info);
	//todo preguntar si habria que hacer algo particular aca...

	res = semMutexEntrada.v(1);
	if ( controlErrores1(res, logger, info) == MUERTE_POR_ERROR) { kill(getppid(),SIGINT);}

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


