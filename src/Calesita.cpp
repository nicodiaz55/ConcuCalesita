//============================================================================
// Name        : CalecitaVpropia.cpp
// Author      : Juan Fuld
// Version     :
// Copyright   : None
// Description : Hello World in C++, Ansi-style
//============================================================================

#ifdef CALESITA

#include "Memoria_Compartida/MemoriaCompartida.h"
#include "Locks/LockWrite.hpp"
#include "Locks/LockRead.hpp"
#include "Semaforos/Semaforo.h"
#include <signal.h>

#include "logger/Logger.hpp"
#include "utils/Utils.hpp"

using namespace std;

/*
 * La calesita
 *
 * */




//todo Control de errores!!

//todo que no loggee si muero == true!
int main(int argc, char** argv) {

	//Abro el logger
	Logger* logger = new Logger();
	Info* info = new Info(getpid(), "Calesita");

	logger->log("Se abre la calesita para este día",info);
	//PARAMETROS
	//tiempoVuelta: Cuanto dura una vuelta de la calesita
	//cantMaxLugares: Cuantos lugares tiene la calesita en total
	//lugaresLibres: cuantos lugares abre para la proxima vuelta
	int tiempoVuelta, cantMaxLugares, lugaresLibres;

	//todo extraer a funcion
	if ( argc != 4 ){
		logger->log("Cantidad de parámetros incorrectos, especifique duración y cantidad de lugares máxima y usados",info);
		kill(getppid(),SIGINT);
	}

	lugaresLibres = toInt(argv[1]);

	cantMaxLugares = toInt(argv[2]);

	tiempoVuelta = toInt(argv[3]);

	//Fin recepcion de parametros

	//obtiene la memoria compartida
	MemoriaCompartida<int> kidsInPark;
	int res = kidsInPark.crear("/etc",33, PERMISOS_USER_RDWR);
	if ( controlErrores1(res, logger, info) == MUERTE_POR_ERROR) { kill(getppid(),SIGINT);}

	MemoriaCompartida<int> continua;
	res = continua.crear("/etc",55, PERMISOS_USER_RDWR);
	if ( controlErrores1(res, logger, info) == MUERTE_POR_ERROR) { kill(getppid(),SIGINT);}

	//obtiene los semaforos para sincronizarse
	Semaforo semCalGira("/etc", 22);
	res = semCalGira.crear();
	if ( controlErrores1(res, logger, info) == MUERTE_POR_ERROR) { kill(getppid(),SIGINT);}

	Semaforo semCalLug("/etc", 23);
	res = semCalLug.crear();
	if ( controlErrores1(res, logger, info) == MUERTE_POR_ERROR) { kill(getppid(),SIGINT);}

	Semaforo semColaCal("/etc", 25);
	res = semColaCal.crear();
	if ( controlErrores1(res, logger, info) == MUERTE_POR_ERROR) { kill(getppid(),SIGINT);}

	//prepara locks de kidsInPark
	LockFile* lockWKidsinPark = new LockWrite("archLockKids");
	LockFile* lockRKidsInPark = new LockRead("archLockKids");
	LockFile* lockRContinua = new LockRead("archLockCont");

	int seguir = 0;

	while (seguir == 0){

		res = semCalLug.zero();//arranca cuando no hay mas lugares
		if ( controlErrores1(res, logger, info) == MUERTE_POR_ERROR) { kill(getppid(),SIGINT);}

		res = lockRContinua->tomarLock();
		if ( controlErrores1(res, logger, info) == MUERTE_POR_ERROR) { kill(getppid(),SIGINT);}

		seguir = continua.leer();

		res = lockRContinua->liberarLock();
		if ( controlErrores1(res, logger, info) == MUERTE_POR_ERROR) { kill(getppid(),SIGINT);}

		if (seguir == 0){
			//no duerme con el fantasma
			logger->log("Arranca una vuelta",info);
			sleep(tiempoVuelta);
			logger->log("Termina la vuelta",info);
		}

		//avisa a los chicos que termino la vuelta
		res = semCalGira.v(lugaresLibres);
		if ( controlErrores1(res, logger, info) == MUERTE_POR_ERROR) { kill(getppid(),SIGINT);}

		//Despues de girar baja la cantidad de chicos esperando
		res = lockWKidsinPark->tomarLock();
		if ( controlErrores1(res, logger, info) == MUERTE_POR_ERROR) { kill(getppid(),SIGINT);}

		int chicosRestantes = kidsInPark.leer() - lugaresLibres;
		kidsInPark.escribir(chicosRestantes);

		res = lockWKidsinPark->liberarLock();
		if ( controlErrores1(res, logger, info) == MUERTE_POR_ERROR) { kill(getppid(),SIGINT);}

		//primero se fija que todos los niños hayan bajado
		res = semCalGira.zero();
		if ( controlErrores1(res, logger, info) == MUERTE_POR_ERROR) { kill(getppid(),SIGINT);}

		//segundo, se bajan los chicos -> abre lugares para los que queden y avisa a puerta que los deje pasar
		res = lockRKidsInPark->tomarLock();
		if ( controlErrores1(res, logger, info) == MUERTE_POR_ERROR) { kill(getppid(),SIGINT);}

		int kidsWaiting = kidsInPark.leer();

		res = lockRKidsInPark->liberarLock();
		if ( controlErrores1(res, logger, info) == MUERTE_POR_ERROR) { kill(getppid(),SIGINT);}

		//segun cuantos chicos estan esperando es la cantidad de lugares que abre
		if (kidsWaiting >= cantMaxLugares){
			lugaresLibres = cantMaxLugares;
		}else{
			lugaresLibres = kidsWaiting;
		}

		//siempre tiene por lo menos 1 lugar libre
		if (lugaresLibres <= 0 ){
			lugaresLibres = 1;
		}

		res = semCalLug.v(lugaresLibres);
		if ( controlErrores1(res, logger, info) == MUERTE_POR_ERROR) { kill(getppid(),SIGINT);}
		res = semColaCal.v(lugaresLibres);
		if ( controlErrores1(res, logger, info) == MUERTE_POR_ERROR) { kill(getppid(),SIGINT);}

	}

	//libero memoria compartida y demas cosas
	res = kidsInPark.liberar();
	if ( controlErrores1(res, logger, info) == MUERTE_POR_ERROR) { kill(getppid(),SIGINT);}

	res = continua.liberar();
	if ( controlErrores1(res, logger, info) == MUERTE_POR_ERROR) { kill(getppid(),SIGINT);}


	delete lockWKidsinPark;
	delete lockRKidsInPark;
	delete lockRContinua;

	logger->log("Se detiene la calesita",info);

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
