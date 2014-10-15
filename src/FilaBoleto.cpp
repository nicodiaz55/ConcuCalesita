/*
 * FilaBolet.cpp
 *
 *  Created on: 14/10/2014
 *      Author: nicolas
 */

#ifdef FILA1

#include "Pipes_y_Fifos/FifoEscritura.h"
#include "Pipes_y_Fifos/Pipe.h"
#include <signal.h>

#include "logger/Logger.hpp"
#include "utils/Utils.hpp"
using namespace std;

 /*
  * Controla las colas de chicos con pipes
  */
class FilaBoleto {
private:
	Logger* logger;
	Info* info;
	pid_t ppid;
	int fdReadKid,fdWriteKid,fdReadPuerta,fdWritePuerta;
	Pipe* pipe1;
	Pipe* pipe2;
	FifoEscritura* fifoRecaudador;
public:
	FilaBoleto(pid_t ppid, pid_t pid) {
		logger = new Logger();
		info = new Info(pid, "FilaBoleto");
		this->ppid = ppid;
		fdReadKid = 0;
		fdWriteKid = 0;
		fdReadPuerta = 0;
		fdWritePuerta = 0;
		pipe1 = NULL;
		pipe2 = NULL;
		fifoRecaudador = NULL;
	}

	void iniciar(char** argv) {
		logger->log("Arranca la fila de venta de boletos",info);

		//recibe pipes
		fdReadKid = toInt(argv[1]);
		fdWriteKid = toInt(argv[2]);
		fdReadPuerta = toInt(argv[3]);
		fdWritePuerta = toInt(argv[4]);

		pipe1 = new Pipe(fdReadKid,fdWriteKid);
		pipe1->setearModo(Pipe::LECTURA);

		pipe2 = new Pipe(fdReadPuerta,fdWritePuerta);
		pipe2->setearModo(Pipe::ESCRITURA);

		fifoRecaudador = new FifoEscritura(FIFORECAUDADOR);
		int res = fifoRecaudador->abrir();
		if (controlErrores2(res, logger, info) == MUERTE_POR_ERROR) {kill(ppid,SIGINT);}
	}

	void trabajar() {
		bool seguir = true;
		while(seguir) {
			int pidKid;
			int res = pipe1->leer(&pidKid, sizeof(int));
			if (res != sizeof(int)){
				logger->log("Atencion: se leyeron del pipe solo: " + toString(res), info);
			}

			if (pidKid != CERRAR_FILA){//-1 llega para que muera la puerta

				string ruta = PREFIJO_FIFO_FILA_KIDS + toString(pidKid);

				//le dice al chico que pase
				FifoEscritura fifoAKid(ruta);
				res = fifoAKid.abrir();
				if ( controlErrores2(res, logger, info) == MUERTE_POR_ERROR) { kill(ppid,SIGINT);}

				logger->log("Le da boleto al chico: " + toString(pidKid),info);

				res = fifoAKid.escribir( &VALOR_PASAR, sizeof(int) );
				if (res != sizeof(int)){
					logger->log("Atencion: se escribieron al pipe solo: " + toString(res), info);
				}

				fifoAKid.cerrar();
				fifoAKid.eliminar();

				//le avisa al recaudador que pago un chico
				int pago = AVISO_DE_PAGO;
				logger->log("Le dice al recaudador que pasó el chico: " + toString(pidKid),info);
				res = fifoRecaudador->escribir( &pago, sizeof(int));
				if (res != sizeof(int)){
					logger->log("Atencion: se escribieron al pipe solo: " + toString(res), info);
				}

				//le mete el niño a la otra fila
				res = pipe2->escribir(&pidKid, sizeof(int));
				if (res != sizeof(int)){
					logger->log("Atencion: se escribieron al pipe solo: " + toString(res), info);
				}
			}else{
				seguir=false;
				//Le pasa el -1 a la otra fila para que muera
				int msj = CERRAR_FILA;
				res = pipe2->escribir(&msj, sizeof(int));
				if (res != sizeof(int)){
					logger->log("Atencion: se escribieron al pipe solo: " + toString(res), info);
				}
			}
		}
	}

	void terminar() {
		logger->log("Se cierra la fila de venta de boletos",info);
		logger->log("Informo al Recaudador que debe terminar", info);
		//para que muera el recaudador
		int pago = FIN_PAGOS;
		int res = fifoRecaudador->escribir( &pago, sizeof(int));
		if (res != sizeof(int)){
			logger->log("Atencion: se escribieron al pipe solo: " + toString(res), info);
		}

		fifoRecaudador->cerrar();
		fifoRecaudador->eliminar();

		pipe1->cerrar();
		pipe2->cerrar();
	}

	~FilaBoleto() {
		delete pipe1;
		delete pipe2;
		delete fifoRecaudador;
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

	FilaBoleto* filaBoleteria = new FilaBoleto(getppid(), getpid());
	filaBoleteria->iniciar(argv);
	filaBoleteria->trabajar();
	filaBoleteria->terminar();
	delete filaBoleteria;
	return 0;
}

#endif
