/*
 * Recaud.cpp
 *
 *  Created on: 13/10/2014
 *      Author: nicolas
 */


#ifdef RECAUDADOR

#include "Pipes_y_Fifos/FifoLectura.h"
#include "Locks/LockWrite.hpp"
#include "Locks/LockRead.hpp"
#include "Semaforos/Semaforo.h"
#include <signal.h>

#include "logger/Logger.hpp"
#include "utils/Utils.hpp"
#include "Caja.h"

using namespace std;

/*
 * Registra en la caja el cobro de boletos
 */
class Recaudador {
private:
	Logger* logger;
	Info* info;
	Caja* caja;
	LockFile* lockW;
	FifoLectura* fifo;
	Semaforo* semAdminRec;
	pid_t ppid;
	int precio;
public:
	Recaudador(pid_t ppid, pid_t pid, int precio) {
		logger = new Logger();
		info = new Info(pid, "Recaudador");
		this->ppid = ppid;
		this->precio = precio;
		caja = NULL;
		lockW = NULL;
		fifo = NULL;
		semAdminRec = NULL;
	}

	void iniciar() {
		logger->log("Entré a trabajar", info);

		fifo = new FifoLectura(FIFORECAUDADOR);
		int res = fifo->abrir();
		if (controlErrores2(res, logger, info) == MUERTE_POR_ERROR) {kill(ppid,SIGINT);}

		//Semaforo para sincronizar con el administrador (caso especial 0 chicos)
		semAdminRec = new Semaforo(ARCH_SEM, SEM_ADMIN_REC);
		res = semAdminRec->crear();
		if (controlErrores1(res, logger, info) == MUERTE_POR_ERROR) {kill(ppid,SIGINT);}

		//pide memoria comp. para caja
		caja = new Caja();
		res = caja->init();
		if (controlErrores1(res, logger, info) == MUERTE_POR_ERROR) {kill(ppid,SIGINT);}

		//prepara lock de caja recaudacion
		lockW = new LockWrite(ARCH_LOCK_CAJA);
	}

	void trabajar() {
		while (true) {
			int avisoPago, res;
			fifo->leer(&avisoPago, sizeof(int));

			// Si termino salgo
			if (avisoPago == FIN_PAGOS) {
				return;
			}

			res = lockW->tomarLock();
			if (controlErrores2(res, logger, info) == MUERTE_POR_ERROR) {kill(ppid,SIGINT);}

			res = caja->aumentarRecaudacion(precio);
			controlErrores1(res, logger, info);

			logger->log("Coloqué $" + toString(precio) + " en la caja", info);

			res = lockW->liberarLock();
			if (controlErrores2(res, logger, info) == MUERTE_POR_ERROR) {kill(ppid,SIGINT);}
		}
	}

	void terminar() {
		//Para que no pase de aca hasta que el administrador tenga tomada la memoria compartida
		semAdminRec->p(-1);

		//para que muera el administrador
		logger->log("Cierro la caja, asi el Administrador sabe que debe terminar", info);
		int res = lockW->tomarLock();
		if (controlErrores2(res, logger, info) == MUERTE_POR_ERROR) {kill(ppid,SIGINT);}

		caja->setearEstado(false);

		res = lockW->liberarLock();
		if (controlErrores2(res, logger, info) == MUERTE_POR_ERROR) {kill(ppid,SIGINT);}

		//libero memoria compartida
		res = caja->terminar();
		if (controlErrores1(res, logger, info) == MUERTE_POR_ERROR) {kill(ppid,SIGINT);}

		fifo->cerrar();
		fifo->eliminar();

		logger->log("Terminé mi trabajo", info);
	}

	~Recaudador() {
		if (caja != NULL) { // TODO: Si se rompe por culpa de caja, es aca.
			delete caja;
			caja = NULL;
		}
		if (lockW != NULL) {
			delete lockW;
			lockW = NULL;
		}
		if (fifo != NULL) {
			delete fifo;
			fifo = NULL;
		}
		if (semAdminRec != NULL) {
			delete semAdminRec;
			semAdminRec = NULL;
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

int main(int argc, char** argv) {
	Recaudador* recaudador = new Recaudador(getppid(), getpid(), toInt(argv[1]));
	recaudador->iniciar();
	recaudador->trabajar();
	recaudador->terminar();
	delete recaudador;
	return 0;
}

#endif


