//============================================================================
// Name        : CalecitaVpropia.cpp
// Author      : Juan Fuld
// Version     :
// Copyright   : None
// Description : Hello World in C++, Ansi-style
//============================================================================

#ifdef CALESITA

#include "Memoria_Compartida/MemoriaCompartida.h"
#include "Memoria_Compartida/VectorMemoCompartida.h"
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
	int res = kidsInPark.crear(ARCH_SEM,MEM_KIDS, PERMISOS_USER_RDWR);
	if ( controlErrores1(res, logger, info) == MUERTE_POR_ERROR) { kill(getppid(),SIGINT);}

	MemoriaCompartida<int> continua;
	res = continua.crear(ARCH_SEM,MEM_CONTINUA, PERMISOS_USER_RDWR);
	if ( controlErrores1(res, logger, info) == MUERTE_POR_ERROR) { kill(getppid(),SIGINT);}

	VectorMemoCompartida<bool> lugares;

	res = lugares.crear(ARCH_SEM,INICIO_CLAVES_LUGARES,PERMISOS_USER_RDWR, cantMaxLugares);
	if ( controlErrores1(res, logger, info) == MUERTE_POR_ERROR ) { kill(getppid(),SIGINT);}

	//obtiene los semaforos para sincronizarse
	Semaforo semCalGira(ARCH_SEM, SEM_CAL_GIRA);
	res = semCalGira.crear();
	if ( controlErrores1(res, logger, info) == MUERTE_POR_ERROR) { kill(getppid(),SIGINT);}

	Semaforo semCalLug(ARCH_SEM, SEM_CAL_LUG);
	res = semCalLug.crear();
	if ( controlErrores1(res, logger, info) == MUERTE_POR_ERROR) { kill(getppid(),SIGINT);}

	Semaforo semCalSubir(ARCH_SEM, SEM_CAL_SUBIR);
	res = semCalSubir.crear();
	if (controlErrores1(res, logger, info) == MUERTE_POR_ERROR) {raise(SIGINT);}

	Semaforo semColaCal(ARCH_SEM, SEM_COLA_CAL);
	res = semColaCal.crear();
	if ( controlErrores1(res, logger, info) == MUERTE_POR_ERROR) { kill(getppid(),SIGINT);}

	//prepara locks
	LockFile* lockWKidsinPark = new LockWrite(ARCH_LOCK_KIDS);
	LockFile* lockRKidsInPark = new LockRead(ARCH_LOCK_KIDS);

	LockFile* lockRContinua = new LockRead(ARCH_LOCK_CONTINUA);

	LockFile* lockSpots = new LockWrite(ARCH_LOCK_LUGARES);

	int seguir = 0;

	while (seguir == 0){

		res = semCalLug.zero();//arranca cuando no hay mas lugares
		if ( controlErrores1(res, logger, info) == MUERTE_POR_ERROR) { kill(getppid(),SIGINT);}

		//se fija si tiene que dar otra vuelta
		res = lockRContinua->tomarLock();
		if ( controlErrores2(res, logger, info) == MUERTE_POR_ERROR) { kill(getppid(),SIGINT);}

		seguir = continua.leer();

		res = lockRContinua->liberarLock();
		if ( controlErrores2(res, logger, info) == MUERTE_POR_ERROR) { kill(getppid(),SIGINT);}

		if (seguir == 0){
			logger->log("Arranca una vuelta",info);
			sleep(tiempoVuelta);
			logger->log("Termina la vuelta",info);
		}

		//avisa a los chicos que termino la vuelta
		res = semCalGira.v(lugaresLibres);
		if ( controlErrores1(res, logger, info) == MUERTE_POR_ERROR) { kill(getppid(),SIGINT);}

		//Despues de girar baja la cantidad de chicos esperando
		res = lockWKidsinPark->tomarLock();
		if ( controlErrores2(res, logger, info) == MUERTE_POR_ERROR) { kill(getppid(),SIGINT);}

		int chicosRestantes = kidsInPark.leer() - lugaresLibres;
		kidsInPark.escribir(chicosRestantes);

		res = lockWKidsinPark->liberarLock();
		if ( controlErrores2(res, logger, info) == MUERTE_POR_ERROR) { kill(getppid(),SIGINT);}

		//primero, se fija que todos los niños hayan bajado
		res = semCalGira.zero();
		if ( controlErrores1(res, logger, info) == MUERTE_POR_ERROR) { kill(getppid(),SIGINT);}

		//segundo, pone todos los lugares en "libre"
		for (int i = 0 ; i < cantMaxLugares; i++){
			lockSpots->tomarLock();
			if ( controlErrores2(res, logger, info) == MUERTE_POR_ERROR ) { kill(getppid(),SIGINT);}

			lugares.escribir(LUGAR_LIBRE,i);

			lockSpots->liberarLock();
			if ( controlErrores2(res, logger, info) == MUERTE_POR_ERROR ) { kill(getppid(),SIGINT);}
		}

		//tercero, se bajan los chicos -> abre lugares para los que queden y avisa a puerta que los deje pasar
		res = lockRKidsInPark->tomarLock();
		if ( controlErrores2(res, logger, info) == MUERTE_POR_ERROR) { kill(getppid(),SIGINT);}

		int kidsWaiting = kidsInPark.leer();

		res = lockRKidsInPark->liberarLock();
		if ( controlErrores2(res, logger, info) == MUERTE_POR_ERROR) { kill(getppid(),SIGINT);}

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
		res = semCalSubir.v(lugaresLibres);
		if ( controlErrores1(res, logger, info) == MUERTE_POR_ERROR) { kill(getppid(),SIGINT);}

	}

	//libero memoria compartida y demas cosas
	res = kidsInPark.liberar();
	if ( controlErrores1(res, logger, info) == MUERTE_POR_ERROR) { kill(getppid(),SIGINT);}

	res = continua.liberar();
	if ( controlErrores1(res, logger, info) == MUERTE_POR_ERROR) { kill(getppid(),SIGINT);}

	res = lugares.liberar();
	if ( controlErrores1(res, logger, info) == MUERTE_POR_ERROR) { kill(getppid(),SIGINT);}

	delete lockWKidsinPark;
	delete lockRKidsInPark;
	delete lockRContinua;
	delete lockSpots;

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
