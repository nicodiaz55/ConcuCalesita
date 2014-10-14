/*
 * KidA.cpp
 *
 *  Created on: 14/10/2014
 *      Author: nicolas
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

using namespace std;

 /*
  * Kid
  *
  * Aumenta KidsInPark al comenzar con shMem y locks
  *	Se mete en la cola para comprar boleto con pipe
  *	Corre para subir a la calesita con semaforo
  *
  */
class KidA {
private:
	Logger* logger;
	Info* info;
	pid_t ppid, pid;
	LockFile* lockKids;
	LockFile* lockSpots;
	Semaforo* semCalGira;
	Semaforo* semMutexEntrada;
	Semaforo* semCalLug;
	Semaforo* semCalSubir;
	int cantlugares;
	MemoriaCompartida<int>* kidsInPark;
	VectorMemoCompartida<bool>* lugares;
	int fdRdPuerta1, fdWrPuerta1;
	Pipe* pipePuerta1;
	FifoLectura* fila;
public:
	KidA(pid_t ppid, pid_t pid) {
		this->ppid = ppid;
		this->pid = pid;
		logger = new Logger();
		info = new Info(pid, "Kid\t");
		lockKids = NULL;
		lockSpots = NULL;
		semCalGira = NULL;
		semMutexEntrada = NULL;
		semCalLug = NULL;
		semCalSubir = NULL;
		cantlugares = 0;
		kidsInPark = NULL;
		lugares = NULL;
		fdRdPuerta1 = 0;
		fdWrPuerta1 = 0;
		pipePuerta1 = NULL;
		fila = NULL;
	}

	void iniciar(char** argv) {
		logger->log("Voy hacia el parque :D", info);

		//recibe pipes y cantidad de lugares
		fdRdPuerta1 = toInt(argv[1]);
		fdWrPuerta1  = toInt(argv[2]);
		cantlugares = toInt(argv[3]);

		pipePuerta1 = new Pipe(fdRdPuerta1,fdWrPuerta1);

		pipePuerta1->setearModo(Pipe::ESCRITURA);

		//para la memoria compartida
		kidsInPark = new MemoriaCompartida<int>();
		int res = kidsInPark->crear(ARCH_SEM,MEM_KIDS, PERMISOS_USER_RDWR);
		if (controlErrores1(res, logger, info) == MUERTE_POR_ERROR) {kill(ppid,SIGINT);}

		lugares = new VectorMemoCompartida<bool>();

		res = lugares->crear(ARCH_SEM,INICIO_CLAVES_LUGARES,PERMISOS_USER_RDWR,cantlugares);
		if (controlErrores1(res, logger, info) == MUERTE_POR_ERROR) {kill(ppid,SIGINT);}

		//Semaforos

		semCalGira = new Semaforo(ARCH_SEM, SEM_CAL_GIRA); // para bajarse
		res = semCalGira->crear();
		if (controlErrores1(res, logger, info) == MUERTE_POR_ERROR) {kill(ppid,SIGINT);}

		semMutexEntrada = new Semaforo(ARCH_SEM, SEM_MUTEX_ENTR); // puerta de entrada/salida
		res = semMutexEntrada->crear();
		if (controlErrores1(res, logger, info) == MUERTE_POR_ERROR) {kill(ppid,SIGINT);}

		semCalLug = new Semaforo(ARCH_SEM, SEM_CAL_LUG); // para subirse
		res = semCalLug->crear();
		if (controlErrores1(res, logger, info) == MUERTE_POR_ERROR) {kill(ppid,SIGINT);}

		semCalSubir = new Semaforo(ARCH_SEM, SEM_CAL_SUBIR);
		res = semCalSubir->crear();
		if (controlErrores1(res, logger, info) == MUERTE_POR_ERROR) {raise(SIGINT);}

		//prepara lock de kidsInPark
		lockKids = new LockWrite(ARCH_LOCK_KIDS);

		//prepara el lock de los lugares
		lockSpots = new LockWrite(ARCH_LOCK_LUGARES);
	}

	void trabajar() {
		//para que entre "de a uno" uso semaforo binario para seccion critica
		int res = semMutexEntrada->p(-1);
		if (controlErrores1(res, logger, info) == MUERTE_POR_ERROR) {kill(ppid,SIGINT);}

		logger->log("Entré al parque", info);

		//como entro aumenta cantidad de chicos presentes...
		lockKids->tomarLock();
		if (controlErrores2(res, logger, info) == MUERTE_POR_ERROR) {kill(ppid,SIGINT);}

		kidsInPark->escribir(kidsInPark->leer() + 1);
		logger->log("Aumento en uno la cantidad de chicos en el parque", info);

		lockKids->liberarLock();
		if (controlErrores2(res, logger, info) == MUERTE_POR_ERROR) {kill(ppid,SIGINT);}

		res = semMutexEntrada->v(1);
		if (controlErrores1(res, logger, info) == MUERTE_POR_ERROR) {kill(ppid,SIGINT);}

		//se mete en la cola de boletos

		logger->log("Me encolo para sacar boleto", info);
	//Meterse es pasarle a la puerta por donde le tiene que escribir para desbloquearlo
		pipePuerta1->escribir(&pid, sizeof(int));

	//Espera que la puerta le escriba "pasa" por el fifo corresponidente
		int permisoPasar = 0;

		string ruta = PREFIJO_FIFO_FILA_KIDS + toString(pid);
		fila = new FifoLectura(ruta);
		res = fila->abrir();
		if (controlErrores2(res, logger, info) == MUERTE_POR_ERROR) {kill(ppid,SIGINT);}

		fila->leer(&permisoPasar, sizeof(int));
	//todo sacar TODOS los numeros magicos, pasar a constantes.h

		logger->log("Obtuve mi boleto ", info);

		//Espera que la segunda puerta le escriba "pasa"
		FifoLectura fila2(ruta + SUFIJO_FIFO_FILA_KIDS);
		res = fila2.abrir();
		if (controlErrores2(res, logger, info) == MUERTE_POR_ERROR) {kill(ppid,SIGINT);}
		res = fila2.leer(&permisoPasar, sizeof(int));

		logger->log("Pasé la cola para subir a la calesita ", info);

		//trata de subir
		res = semCalSubir->p(-1);
		if (controlErrores1(res, logger, info) == MUERTE_POR_ERROR) {kill(ppid,SIGINT);}

		logger->log("Me subí a la calesita :D", info);

		//Cuando puede subirse intenta ir al lugar que quiere
		bool libre;

		for (int i = 0 ; i < cantlugares; i++) {
			lockSpots->tomarLock();
			if (controlErrores2(res, logger, info) == MUERTE_POR_ERROR) {kill(ppid,SIGINT);}

			libre = lugares->leer(i);
			if (libre == LUGAR_LIBRE){
				logger->log("Me senté en el lugar: " + toString(i), info);
				lugares->escribir(LUGAR_OCUPADO,i);

				lockSpots->liberarLock();
				if (controlErrores2(res, logger, info) == MUERTE_POR_ERROR) {kill(ppid,SIGINT);}

				break;
			} else {
				string msj = "Quize el lugar: " + toString(i) + " ,pero estaba ocupado.";
				logger->log(msj, info);
			}

			lockSpots->liberarLock();
			if (controlErrores2(res, logger, info) == MUERTE_POR_ERROR) {kill(ppid,SIGINT);}
		}

		//termina de subir a la calesita (lleno su lugar)
		res = semCalLug->p(-1);
		if (controlErrores1(res, logger, info) == MUERTE_POR_ERROR) {kill(ppid,SIGINT);}


		logger->log("Obtuve lugar.", info);

		//espera que la calesita gire
		res = semCalGira->p(-1);
		if (controlErrores1(res, logger, info) == MUERTE_POR_ERROR) {kill(ppid,SIGINT);}

		logger->log("Me bajé de la calesita :(", info);
	}

	void terminar() {
		//libero memoria compartida
		int res = kidsInPark->liberar();
		if (controlErrores1(res, logger, info) == MUERTE_POR_ERROR) {kill(ppid,SIGINT);}

		res = lugares->liberar();
		if (controlErrores1(res, logger, info) == MUERTE_POR_ERROR) {kill(ppid,SIGINT);}
	  	//cierro pipe a las puerta y fifo de la cola
		pipePuerta1->cerrar();

		fila->cerrar();
		fila->eliminar();

		//para que salga "de a uno" uso semaforo binario para seccion critica, uso el mismo de la entrada
		// por fiaca de no crear otro mas, a lo sumo no hay chicos entrando al mismo tiempo que otros salen

		res = semMutexEntrada->p(-1);
		if (controlErrores1(res, logger, info) == MUERTE_POR_ERROR) {kill(ppid,SIGINT);}

		logger->log("Me fui del parque", info);

		res = semMutexEntrada->v(1);
		if (controlErrores1(res, logger, info) == MUERTE_POR_ERROR) {kill(ppid,SIGINT);}
	}

	~KidA() {
		if (logger != NULL) {
			delete logger;
			logger = NULL;
		}
		if (info != NULL) {
			delete info;
			info = NULL;
		}
		delete lockKids;
		delete lockSpots;
		delete semCalGira;
		delete semMutexEntrada;
		delete semCalLug;
		delete semCalSubir;
		delete kidsInPark;
		delete lugares;
		delete pipePuerta1;
		delete fila;
	}
};


int main (int argc, char** argv) {
	KidA* kid = new KidA(getppid(), getpid());
	kid->iniciar(argv);
	kid->trabajar();
	kid->terminar();
	delete kid;
	return 0;
}

#endif
