/*
 * Cales.cpp
 *
 *  Created on: 14/10/2014
 *      Author: nicolas
 */

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
class Calesita {
private:
	Logger* logger;
	Info* info;
	pid_t ppid;
	MemoriaCompartida<int>* kidsInPark;
	VectorMemoCompartida<bool>* lugares;
	MemoriaCompartida<int>* continua;
	Semaforo* semCalGira;
	Semaforo* semCalLug;
	Semaforo* semCalSubir;
	Semaforo* semColaCal;
	LockFile* lockWKidsinPark;
	LockFile* lockRKidsInPark;
	LockFile* lockRContinua;
	LockFile* lockSpots;
	int tiempoVuelta, cantMaxLugares, lugaresLibres;
public:

	Calesita(pid_t ppid, pid_t pid) {
		this->ppid = ppid;
		logger = new Logger();
		info = new Info(pid, "Calesita");
		kidsInPark = NULL;
		lugares = NULL;
		continua = NULL;
		semCalGira = NULL;
		semCalLug = NULL;
		semCalSubir = NULL;
		semColaCal = NULL;
		lockWKidsinPark = NULL;
		lockRKidsInPark = NULL;
		lockRContinua = NULL;
		lockSpots = NULL;
		tiempoVuelta = 0;
		cantMaxLugares = 0;
		lugaresLibres = 0;
	}

	void iniciar(int argc, char** argv) {
		logger->log("Se abre la calesita para este día",info);
		//PARAMETROS
		//tiempoVuelta: Cuanto dura una vuelta de la calesita
		//cantMaxLugares: Cuantos lugares tiene la calesita en total
		//lugaresLibres: cuantos lugares abre para la proxima vuelta
		if ( argc != 4 ){
			logger->log("Cantidad de parámetros incorrectos, especifique duración y cantidad de lugares máxima y usados",info);
			kill(ppid,SIGINT);
		}

		lugaresLibres = toInt(argv[1]);
		cantMaxLugares = toInt(argv[2]);
		tiempoVuelta = toInt(argv[3]);

		//Fin recepcion de parametros

		//obtiene la memoria compartida
		kidsInPark = new MemoriaCompartida<int>;
		int res = kidsInPark->crear(ARCH_SEM,MEM_KIDS, PERMISOS_USER_RDWR);
		if ( controlErrores1(res, logger, info) == MUERTE_POR_ERROR) { kill(ppid,SIGINT);}

		continua = new MemoriaCompartida<int>;
		res = continua->crear(ARCH_SEM,MEM_CONTINUA, PERMISOS_USER_RDWR);
		if ( controlErrores1(res, logger, info) == MUERTE_POR_ERROR) { kill(ppid,SIGINT);}

		lugares = new VectorMemoCompartida<bool>;

		res = lugares->crear(ARCH_SEM,INICIO_CLAVES_LUGARES,PERMISOS_USER_RDWR, cantMaxLugares);
		if ( controlErrores1(res, logger, info) == MUERTE_POR_ERROR ) { kill(ppid,SIGINT);}

		//obtiene los semaforos para sincronizarse
		semCalGira = new Semaforo(ARCH_SEM, SEM_CAL_GIRA);
		res = semCalGira->crear();
		if ( controlErrores1(res, logger, info) == MUERTE_POR_ERROR) { kill(ppid,SIGINT);}

		semCalLug = new Semaforo(ARCH_SEM, SEM_CAL_LUG);
		res = semCalLug->crear();
		if ( controlErrores1(res, logger, info) == MUERTE_POR_ERROR) { kill(ppid,SIGINT);}

		semCalSubir = new Semaforo(ARCH_SEM, SEM_CAL_SUBIR);
		res = semCalSubir->crear();
		if (controlErrores1(res, logger, info) == MUERTE_POR_ERROR) {raise(SIGINT);}

		semColaCal = new Semaforo(ARCH_SEM, SEM_COLA_CAL);
		res = semColaCal->crear();
		if ( controlErrores1(res, logger, info) == MUERTE_POR_ERROR) { kill(ppid,SIGINT);}

		//prepara locks
		lockWKidsinPark = new LockWrite(ARCH_LOCK_KIDS);
		lockRKidsInPark = new LockRead(ARCH_LOCK_KIDS);
		lockRContinua = new LockRead(ARCH_LOCK_CONTINUA);
		lockSpots = new LockWrite(ARCH_LOCK_LUGARES);
	}

	void trabajar() {
		int seguir = 0;
		int res;
		while (seguir == 0){

			res = semCalLug->zero();//arranca cuando no hay mas lugares
			if ( controlErrores1(res, logger, info) == MUERTE_POR_ERROR) { kill(ppid,SIGINT);}

			//se fija si tiene que dar otra vuelta
			res = lockRContinua->tomarLock();
			if ( controlErrores2(res, logger, info) == MUERTE_POR_ERROR) { kill(ppid,SIGINT);}

			seguir = continua->leer();

			res = lockRContinua->liberarLock();
			if ( controlErrores2(res, logger, info) == MUERTE_POR_ERROR) { kill(ppid,SIGINT);}

			if (seguir == 0){
				logger->log("Arranca una vuelta",info);
				sleep(tiempoVuelta);
				logger->log("Termina la vuelta",info);
			}

			//avisa a los chicos que termino la vuelta
			res = semCalGira->v(lugaresLibres);
			if ( controlErrores1(res, logger, info) == MUERTE_POR_ERROR) { kill(ppid,SIGINT);}

			//Despues de girar baja la cantidad de chicos esperando
			res = lockWKidsinPark->tomarLock();
			if ( controlErrores2(res, logger, info) == MUERTE_POR_ERROR) { kill(ppid,SIGINT);}

			int chicosRestantes = kidsInPark->leer() - lugaresLibres;
			kidsInPark->escribir(chicosRestantes);

			res = lockWKidsinPark->liberarLock();
			if ( controlErrores2(res, logger, info) == MUERTE_POR_ERROR) { kill(ppid,SIGINT);}

			//primero, se fija que todos los niños hayan bajado
			res = semCalGira->zero();
			if ( controlErrores1(res, logger, info) == MUERTE_POR_ERROR) { kill(ppid,SIGINT);}

			//segundo, pone todos los lugares en "libre"
			for (int i = 0 ; i < cantMaxLugares; i++){
				lockSpots->tomarLock();
				if ( controlErrores2(res, logger, info) == MUERTE_POR_ERROR ) { kill(ppid,SIGINT);}

				lugares->escribir(LUGAR_LIBRE,i);

				lockSpots->liberarLock();
				if ( controlErrores2(res, logger, info) == MUERTE_POR_ERROR ) { kill(ppid,SIGINT);}
			}

			//tercero, se bajan los chicos -> abre lugares para los que queden y avisa a puerta que los deje pasar
			res = lockRKidsInPark->tomarLock();
			if ( controlErrores2(res, logger, info) == MUERTE_POR_ERROR) { kill(ppid,SIGINT);}

			int kidsWaiting = kidsInPark->leer();

			res = lockRKidsInPark->liberarLock();
			if ( controlErrores2(res, logger, info) == MUERTE_POR_ERROR) { kill(ppid,SIGINT);}

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

			res = semCalLug->v(lugaresLibres);
			if ( controlErrores1(res, logger, info) == MUERTE_POR_ERROR) { kill(ppid,SIGINT);}
			res = semColaCal->v(lugaresLibres);
			if ( controlErrores1(res, logger, info) == MUERTE_POR_ERROR) { kill(ppid,SIGINT);}
			res = semCalSubir->v(lugaresLibres);
			if ( controlErrores1(res, logger, info) == MUERTE_POR_ERROR) { kill(ppid,SIGINT);}

		}
	}

	void terminar() {
		//libero memoria compartida y demas cosas
		int res = kidsInPark->liberar();
		if ( controlErrores1(res, logger, info) == MUERTE_POR_ERROR) { kill(ppid,SIGINT);}

		res = continua->liberar();
		if ( controlErrores1(res, logger, info) == MUERTE_POR_ERROR) { kill(ppid,SIGINT);}

		res = lugares->liberar();
		if ( controlErrores1(res, logger, info) == MUERTE_POR_ERROR) { kill(ppid,SIGINT);}

		logger->log("Se detiene la calesita",info);
	}

	~Calesita() {
		delete kidsInPark;
		delete lugares;
		delete continua;
		delete semCalGira;
		delete semCalLug;
		delete semCalSubir;
		delete semColaCal;
		delete lockWKidsinPark;
		delete lockRKidsInPark;
		delete lockRContinua;
		delete lockSpots;
		if (logger != NULL) {
			delete logger;
			logger = NULL;
		}
		if (info != NULL) {
			delete info;
			info = NULL;
		}
	}
};


int main(int argc, char** argv) {
	Calesita* calesita = new Calesita(getppid(), getpid());
	calesita->iniciar(argc, argv);
	calesita->trabajar();
	calesita->terminar();
	delete calesita;
	return 0;
}

#endif
