/*
 * Fantas.cpp
 *
 *  Created on: 14/10/2014
 *      Author: nicolas
 */


#ifdef FANTASMA

#include "Semaforos/Semaforo.h"
#include "Memoria_Compartida/MemoriaCompartida.h"
#include "Locks/LockWrite.hpp"
#include <signal.h>
#include "Constantes.h"
#include "logger/Logger.hpp"
#include "utils/Utils.hpp"

using namespace std;

 /*
  * Kid fantasma para cerrar la calesita y que no quede bloqueada por semaforos
  *
  */
class Fantasma {
private:
	Logger* logger;
	Info* info;
	MemoriaCompartida<int>* continua;
	Semaforo* semCalGira;
	Semaforo* semCalLug;
	LockFile* lockWContinua;
	pid_t ppid;
public:
	Fantasma(pid_t ppid, pid_t pid) {
		logger = new Logger();
		info = new Info(pid, "Fantasma");
		continua = NULL;
		semCalGira = NULL;
		semCalLug = NULL;
		lockWContinua = NULL;
		this->ppid = ppid;
	}

	void iniciar() {
		logger->log("Entré al parque", info);

		//para la memoria compartida
		continua = new MemoriaCompartida<int>();
		int res = continua->crear(ARCH_SEM, MEM_CONTINUA, PERMISOS_USER_RDWR);
		if (controlErrores1(res, logger, info) == MUERTE_POR_ERROR) {kill(ppid,SIGINT);}

		//semaforos
		semCalGira = new Semaforo(ARCH_SEM, SEM_CAL_GIRA);
		res = semCalGira->crear();
		if (controlErrores1(res, logger, info) == MUERTE_POR_ERROR) {kill(ppid,SIGINT);}

		semCalLug = new Semaforo(ARCH_SEM, SEM_CAL_LUG);
		res = semCalLug->crear();
		if (controlErrores1(res, logger, info) == MUERTE_POR_ERROR) {kill(ppid,SIGINT);}

		//prepara lock de continua
		lockWContinua = new LockWrite(ARCH_LOCK_CONTINUA);
	}

	void trabajar() {
		int res = lockWContinua->tomarLock();
		if (controlErrores1(res, logger, info) == MUERTE_POR_ERROR) {kill(ppid,SIGINT);}

		continua->escribir(CERRAR);

		res = lockWContinua->liberarLock();
		if (controlErrores1(res, logger, info) == MUERTE_POR_ERROR) {kill(ppid,SIGINT);}

		//intenta subir a la calesita
		logger->log("Intento subir a la calesita", info);

		res = semCalLug->p(-1);
		if (controlErrores1(res, logger, info) == MUERTE_POR_ERROR) {kill(ppid,SIGINT);}

		logger->log("Subí a la calesita", info);

		//espera que la calesita gire
		res = semCalGira->p(-1);
		if (controlErrores1(res, logger, info) == MUERTE_POR_ERROR) {kill(ppid,SIGINT);}

		logger->log("Me bajé de la calesita", info);
	}

	void terminar() {
		//libero memoria compartida
		int res = continua->liberar();
		if (controlErrores1(res, logger, info) == MUERTE_POR_ERROR) {kill(ppid,SIGINT);}

		logger->log("Como mis deseos infantiles fueron satisfecho, me retiro de este mundo", info);
	}

	~Fantasma() {
		if (continua != NULL) {
			delete continua;
			continua = NULL;
		}
		if (semCalGira != NULL) {
			delete semCalGira;
			semCalGira = NULL;
		}
		if (semCalLug != NULL) {
			delete semCalLug;
			semCalLug = NULL;
		}
		if (lockWContinua != NULL) {
			delete lockWContinua;
			lockWContinua = NULL;
		}
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

int main ( int argc, char** argv){
	Fantasma* fantasma = new Fantasma(getppid(), getpid());
	fantasma->iniciar();
	fantasma->trabajar();
	fantasma->terminar();
	delete fantasma;
  	return 0;
}

#endif
