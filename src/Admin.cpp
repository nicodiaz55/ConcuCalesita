/*
 * Admin.cpp
 *
 *  Created on: 13/10/2014
 *      Author: nicolas
 */

#ifdef ADMIN

#include "Locks/LockRead.hpp"
#include "Semaforos/Semaforo.h"
#include <signal.h>
#include "utils/Random.hpp"
#include "logger/Logger.hpp"
#include "utils/Utils.hpp"
#include "Caja.h"

using namespace std;

/*
 * Mira la caja de boletos e imprime por pantalla o al log o whatever
 */
class Admin {
	private:
		Logger* logger;
		Info* info;
		pid_t ppid;
		Caja* caja;
		LockFile* lockR;
	public:
		Admin(pid_t ppid, pid_t pid) {
			logger = new Logger();
			info = new Info(pid, "Administrador");
			this->ppid = ppid;
			caja = NULL;
			lockR = NULL;
		}

		void iniciar() {
			//Semaforo para sincronizar con el recaudador (caso especial 0 chicos)
			Semaforo semAdminRec(ARCH_SEM, SEM_ADMIN_REC);
			int res = semAdminRec.crear();
			if (controlErrores1(res, logger, info) == MUERTE_POR_ERROR) {kill(ppid,SIGINT);}

			logger->log("Entré a trabajar", info);

			//pide memoria comp. para caja
			caja = new Caja();
			res = caja->init();
			if (controlErrores1(res, logger, info) == MUERTE_POR_ERROR) {kill(ppid,SIGINT);}

			//ahora puede terminar el recaudador, si no sigue de largo y nunca me dice que termine
			semAdminRec.v(1);

			//prepara lock de caja recaudacion
			lockR = new LockRead(ARCH_LOCK_CAJA);
		}

		void trabajar(int min, int max) {
			int res;
			while (true){
				res = lockR->tomarLock();
				if (controlErrores2(res, logger, info) == MUERTE_POR_ERROR) {kill(ppid,SIGINT);}

				logger->log("En la caja hay: $" + toString(caja->obtenerRecaudacion()), info);
				if (caja->obtenerEstado() == false) {
					break;
				}
				res = lockR->liberarLock();
				if (controlErrores2(res, logger, info) == MUERTE_POR_ERROR) {kill(ppid,SIGINT);}

				//este sleep esta para que no llene el log solo con sus lecturas
				sleep(uniform(min,max));
			}
		}

		void terminar() {
			//libero memoria compartida
			int res = caja->terminar();
			if (controlErrores1(res, logger, info) == MUERTE_POR_ERROR) {kill(ppid,SIGINT);}

			logger->log("Terminé mi trabajo", info);
		}

		~Admin() {
			if (lockR != NULL) {
				delete lockR;
				lockR = NULL;
			}
			if (logger != NULL) {
				delete logger;
				logger = NULL;
			}
			if (info != NULL) {
				delete info;
				info = NULL;
			}
			if (caja != NULL) { // todo: si algo llega a romper con caja, es esto.
				delete caja;
				caja = NULL;
			}
		}
};

int main(int argc, char** argv) {
	Admin* admin = new Admin(getppid(), getpid());
	admin->iniciar();
	// Trabaja viendo la caja cada Uniforme[1,2] segundos
	admin->trabajar(1,2);  // TODO: jugar con esto
	admin->terminar();
	delete admin;
	return 0;
}

#endif