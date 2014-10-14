/*
 * FilaCales.cpp
 *
 *  Created on: 14/10/2014
 *      Author: nicolas
 */

#ifdef FILA2

#include "Pipes_y_Fifos/FifoEscritura.h"
#include "Pipes_y_Fifos/Pipe.h"
#include "Semaforos/Semaforo.h"
#include <signal.h>

#include "logger/Logger.hpp"
#include "utils/Utils.hpp"

using namespace std;

 /*
  * Controla la cola de chicos despues de que tengan boleto
  */
class FilaCalesita {
private:
	Logger* logger;
	Info* info;
	pid_t ppid;
	Pipe* pipeEntrePuertas;
	Semaforo* semColaCal;
public:
	FilaCalesita(pid_t ppid, pid_t pid) {
		this->ppid = ppid;
		logger = new Logger();
		info = new Info(pid, "Fila Calesita");
		pipeEntrePuertas = NULL;
		semColaCal = NULL;
	}

	void iniciar(char** argv) {
		logger->log("Arranca la fila de la calesita",info);

		//recibe pipes
		int fdReadPuerta,fdWritePuerta;

		fdReadPuerta = toInt(argv[1]);
		fdWritePuerta  = toInt(argv[2]);

		pipeEntrePuertas = new Pipe(fdReadPuerta,fdWritePuerta);
		pipeEntrePuertas->setearModo(Pipe::LECTURA);

		//agarra semaforo
		semColaCal = new Semaforo(ARCH_SEM, SEM_COLA_CAL);
		int res = semColaCal->crear();
		if ( controlErrores1(res, logger, info) == MUERTE_POR_ERROR) { kill(ppid,SIGINT);}
	}

	void trabajar() {
		bool seguir = true;

		while (seguir){
			int pidKid, res;
			pipeEntrePuertas->leer(&pidKid, sizeof(int));


			if (pidKid != -1) {
				string ruta = PREFIJO_FIFO_FILA_KIDS + toString(pidKid);

				//le dice al chico que pase. No hay problema con usar el mismo fifo en ambas filas.
				//para que no deje pasar a mas de los que pueden subir "considera" cuantos quiere la calesita
				res = semColaCal->p(-1);
				if ( controlErrores1(res, logger, info) == MUERTE_POR_ERROR) { kill(ppid,SIGINT);}

				FifoEscritura fifoAKid(ruta + SUFIJO_FIFO_FILA_KIDS);
				res = fifoAKid.abrir();
				if ( controlErrores2(res, logger, info) == MUERTE_POR_ERROR) { kill(ppid,SIGINT);}

				logger->log("Pasa el chico: " + toString(pidKid),info);

				fifoAKid.escribir( &VALOR_PASAR2, sizeof(int) );

				fifoAKid.cerrar();
				fifoAKid.eliminar();
			}else{
				seguir = false;
			}
		}
	}

	void terminar() {
		pipeEntrePuertas->cerrar();
		logger->log("Se cierra la fila de la calesita",info);
	}

	~FilaCalesita() {
		delete pipeEntrePuertas;
		delete semColaCal;
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
	FilaCalesita* filaCalesita = new FilaCalesita(getppid(), getpid());
	filaCalesita->iniciar(argv);
	filaCalesita->trabajar();
	filaCalesita->terminar();
	delete filaCalesita;
	return 0;
}

#endif
