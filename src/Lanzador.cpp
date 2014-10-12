/*
 * Lanzador.cpp
 *
 *  Created on: 12/10/2014
 *      Author: nicolas
 */

#include "Lanzador.h"
#include "Seniales/SignalHandler.h"
#include "Seniales/SIGINT_Handler.h"
#include "Semaforos/Semaforo.h"
#include "Pipes_y_Fifos/Pipe.h"
#include <sys/wait.h>
#include <cstdlib>
#include <cerrno>
#include "Constantes.h"
#include "Caja.h"
#include "utils/Utils.hpp"
#include "utils/Random.hpp"

using namespace std;

Lanzador::Lanzador(int cantNinios, int lugaresCalesita, int tiempoVuelta) {
	logger = new Logger();
	info = new Info(getpid(), "Lanzador");
	this->cantNinios = cantNinios;
	this->lugaresCalesita = lugaresCalesita;
	this->tiempoVuelta = tiempoVuelta;
	auxLugares = 0;
	sigint_handler = NULL;
	semCalLug = NULL;
	semColaCal = NULL;
	semCalGira = NULL;
	semMutexEntrada = NULL;
	fdWrPuerta1 = 0;
	pipeAKids = NULL;
	logger->start();
	logger->log("El tiempo de vuelta es de: " + toString(tiempoVuelta)	+ " segundos", info);
	logger->log("La cantidad de lugares es: " + toString(lugaresCalesita),	info);
	logger->log("La cantidad de niños en el barrio es: " + toString(cantNinios), info);
}

void Lanzador::iniciar() {
	logger->log("Empieza la simulación", info);

	//pongo el manejador de la señal
	sigint_handler = new SIGINT_Handler();
	int res = SignalHandler::getInstance()->registrarHandler(SIGINT, sigint_handler);
	if (res != RES_OK) {
		logger->log("Error: " + toString(res) + ". Strerr: " + toString(strerror(errno)),info);
		raise(SIGINT);
	}

	//INICIALIZAR IPC mechanisms

	//INICIALIZA SEMAFOROS
	semCalGira = new Semaforo("/etc", 22, 0); //indica si la calesita esta girando, 0 esta girando, >0 cant gente que puede bajar
	res = semCalGira->crear();
	if (controlErrores1(res, logger, info) == MUERTE_POR_ERROR) {raise(SIGINT);}

	semMutexEntrada = new Semaforo("/etc", 24, 1); //indica si se puede entrar o no al parque (para que los niños "entren y salgan de a uno")
	res = semMutexEntrada->crear();
	if (controlErrores1(res, logger, info) == MUERTE_POR_ERROR) {raise(SIGINT);}

	auxLugares = 0;

	//si los chicos no alcanzan a llenarla solo espera a que se sienten todos para arrancar
	if (cantNinios < lugaresCalesita) {
		auxLugares = cantNinios;
	} else {
		auxLugares = lugaresCalesita;
	}

	semCalLug = new Semaforo("/etc", 23, auxLugares); //indica la cantidad de lugares disponibles en calesita
	semColaCal = new Semaforo("/etc", 25, auxLugares); //indica cuantos niños pueden pasar la cola para subir a la calesita

	res = semCalLug->crear();
	if (controlErrores1(res, logger, info) == MUERTE_POR_ERROR) {raise(SIGINT);}

	res = semColaCal->crear();
	if (controlErrores1(res, logger, info) == MUERTE_POR_ERROR) {raise(SIGINT);}
}

void Lanzador::lanzarAdministradorYRecaudador() {
	//Memoria compartida para la caja
	Caja caja;
	int res = caja.init();
	caja.iniciarRecaudacion();
	if (controlErrores1(res, logger, info) == MUERTE_POR_ERROR) {raise(SIGINT);}

	pid_t pidAdmin = fork();
	if (pidAdmin == -1) {
		logger->log("Error: Falla en fork del administrador. Strerr: " + toString(strerror(errno)), info);
		raise (SIGINT);
	}
	if (pidAdmin == 0) {
		int res = execl("Administrador", "Administrador", (char*) 0);
		if (res != RES_OK) {
			logger->log("Error: Falla en exec del administrador. Strerr: " + toString(strerror(errno)), info);
			raise (SIGINT);
		}
	}
	logger->log("Se lanzó el administrador", info);

	pid_t pidRec = fork();
	if (pidRec == -1) {
		logger->log("Error: Falla fork de recaudador. Strerr: " + toString(strerror(errno)), info);
		raise (SIGINT);
	}
	if (pidRec == 0) {
		int res = execl("Recaudador", "Recaudador", (char*) 0);
		if (res != RES_OK) {
			logger->log("Error: Falla en exec del recaudador. Strerr: " + toString(strerror(errno)), info);
			raise (SIGINT);
		}
	}
	logger->log("Se lanzó el recaudador", info);

	//desattachea de la caja una vez que la tienen Recaud. y Admin.
	int res = caja.terminar();
	if (controlErrores1(res, logger, info) == MUERTE_POR_ERROR) {raise(SIGINT);}
}

void Lanzador::lanzarCalesita() {
	//Memoria compartida para la calesita
	MemoriaCompartida<int> continua;
	int res = continua.crear("/etc", 55, PERMISOS_USER_RDWR);
	if (controlErrores1(res, logger, info) == MUERTE_POR_ERROR) {raise(SIGINT);}

	continua.escribir(0);

	//Memoria compartida para identificar los lugares de la calesita
	int claveAux = INICIO_CLAVES_LUGARES;

	std::vector<MemoriaCompartida<bool> > memLugares;

	for (int i = 0; i < lugaresCalesita; i++){
		MemoriaCompartida<bool> lugar;
		res = lugar.crear("/home/juan/git/ConcuCalesita/src/Constantes.h", claveAux, PERMISOS_USER_RDWR);
		if (controlErrores1(res, logger, info) == MUERTE_POR_ERROR) {raise(SIGINT);}

		lugar.escribir(LUGAR_LIBRE);

		claveAux++;
		cout << "Creo y meto: " << lugar.shmId << endl;
		memLugares.push_back(lugar);
	}

	string arg1 = toString(auxLugares);
	string arg2 = toString(lugaresCalesita);
	string arg3 = toString(tiempoVuelta);

	pid_t pidCal;
	pidCal = fork();
	if (pidCal == -1){
		logger->log("Error: Falla en fork de la calesita. Strerr: " + toString(strerror(errno)), info);
		raise (SIGINT);
	}
	if (pidCal == 0) {
		res = execl("Calesita", "Calesita", arg1.c_str(), arg2.c_str(), arg3.c_str(),
				(char*) 0);
		if (res != RES_OK){
			logger->log("Error: Falla en exec de la calesita. Strerr: " + toString(strerror(errno)), info);
			raise (SIGINT);
		}
	}

	logger->log("Se lanzó la calesita", info);

	//libero para que ya la maneje solo la calesita (y mas tarde el fantasma)
	res = continua.liberar();
	if (controlErrores1(res, logger, info) == MUERTE_POR_ERROR) {raise(SIGINT);}

	//desattacheo tambien de la de los lugares
	vector<MemoriaCompartida<bool> >::iterator it ;

	for ( it = memLugares.begin(); it != memLugares.end(); it++) {
		res = (*it).liberar();
		cout << "Libero: " << (*it).shmId << endl;
		if ( controlErrores1(res, logger, info) == MUERTE_POR_ERROR ) { raise (SIGINT);}
	}
}

void Lanzador::lanzarFilasYNinios() {
	Pipe pipeEntrePuertas; // entre la fila de boletos y la de la calesita
	int res = pipeEntrePuertas.crear();
	if (controlErrores1(res, logger, info) == MUERTE_POR_ERROR) {raise(SIGINT);}

	int fdRdPuerta2 = pipeEntrePuertas.getFdLectura(); // puerta de la calesita
	int fdWrPuerta2 = pipeEntrePuertas.getFdEscritura(); //para escribirle a la puerta 2

	pid_t pidPuerta2 = fork();
	if (pidPuerta2 == -1) {
		logger->log("Error: Falla en fork de la fila de la calesita. Strerr: " + toString(strerror(errno)), info);
		raise(SIGINT);
	}
	if (pidPuerta2 == 0) {
		res = execl("FilaCalesita", "Puerta2", toString(fdRdPuerta2).c_str(), toString(fdWrPuerta2).c_str(), (char*) 0);
		if (res != RES_OK) {
			logger->log("Error: Falla en exec de la fila de la calesita. Strerr: " + toString(strerror(errno)), info);
			raise(SIGINT);
		}
	}

	logger->log("Se lanzó la fila para subir a la calesita", info);

	//pipe para que los ninios se encolen para boleto
	pipeAKids = new Pipe();
	res = pipeAKids->crear();
	if (controlErrores1(res, logger, info) == MUERTE_POR_ERROR) {raise(SIGINT);}

	int fdRdPuerta1 = pipeAKids->getFdLectura();
	fdWrPuerta1 = pipeAKids->getFdEscritura(); //para escribirle a la puerta 1

	pid_t pidPuerta1 = fork();
	if (pidPuerta1 == -1) {
		logger->log("Error: Falla en fork de la fila de boletos. Strerr: " + toString(strerror(errno)), info);
		raise(SIGINT);
	}

	if (pidPuerta1 == 0) {
		res = execl("FilaBoleto", "Fila1", toString(fdRdPuerta1).c_str(),
				toString(fdWrPuerta1).c_str(), toString(fdRdPuerta2).c_str(),
				toString(fdWrPuerta2).c_str(), (char*) 0);
		if (res != RES_OK) {
			logger->log("Error: Falla en exec de la fila de boletos. Strerr: " + toString(strerror(errno)), info);
			raise(SIGINT);
		}
	}

	logger->log("Se lanzó la fila para comprar boleto", info);

	pipeEntrePuertas.cerrar();

	MemoriaCompartida<int> kidsInPark;
	int res = kidsInPark.crear("/etc", 33, PERMISOS_USER_RDWR);
	if (controlErrores1(res, logger, info) == MUERTE_POR_ERROR) {raise(SIGINT);}

	//Memoria compartida de cantidad de chicos en parque
	kidsInPark.escribir(0);

	//lanza niños
	for (int i = 0; i < cantNinios; i++) {

		pid_t pid = fork();
		if (pid == -1){
			logger->log("Error: Falla en fork de un niño. Strerr: " + toString(strerror(errno)), info);
			raise (SIGINT);
		}

		if (pid == 0) {
			res = execl("Kid", "Kid", toString(fdRdPuerta1).c_str(),
					toString(fdWrPuerta1).c_str(), toString(lugaresCalesita).c_str(), (char*) 0);
			if (res != RES_OK){
				logger->log("Error: Falla en exec de un niño. Strerr: " + toString(strerror(errno)), info);
				raise (SIGINT);
			}
		}

		logger->log("Se lanzó un niño", info);
		usleep((int) exponentialTime(2));
	}

	//se desattachea esta shared mem. despues de crear los hijos, asi siempre hay alguien usandola.
	res = kidsInPark.liberar();
	if ( controlErrores1(res, logger, info) == MUERTE_POR_ERROR ) { raise (SIGINT);}
}

void Lanzador::terminar() {
	//espero que terminen todos los hijos
	int status, res;
	for (int i = 0; i < cantNinios; i++) {
		res = wait(&status);
		if (res == -1){
			logger->log("Error: en el wait de un chico . Strerr: " + toString(strerror(errno)), info);
			raise (SIGINT);
		}
	}

	//Le dice a las puertas que mueran

	//espera a las puertas

	int aux = -1;
	res = write(fdWrPuerta1, &aux, sizeof(int));
	if (res == -1){
		logger->log("Error: en un write. Strerr: " + toString(strerror(errno)), info);
		raise (SIGINT);
	}

	//cierro descriptor
	//lo tengo abierto hasta aca porque igual no hay forks que los dupliquen en otros procesos
	//y porque lo necesito para matar la puerta
	pipeAKids->cerrar();
	delete pipeAKids;

	res = wait(&status); //todo waitpid
	if (res == -1){
		logger->log("Error: en el wait puerta 1. Strerr: " + toString(strerror(errno)), info);
		raise (SIGINT);
	}

	//para destrabar la puerta 2
	res = semColaCal->v(1);
	if ( controlErrores1(res, logger, info) == MUERTE_POR_ERROR ) { raise (SIGINT);}

	res = wait(&status);
	if (res == -1){
		logger->log("Error: en el wait puerta 2. Strerr: " + toString(strerror(errno)), info);
		raise (SIGINT);
	}
	//espera a la calesita
	//puede que la calesita quede bloqueada esperando a un ultimo niño-> mando un fantasma

	//hace que un "chico fantasma" de la ultima vuelta
	pid_t pidFantasma = fork();
	if (pidFantasma == -1){
		logger->log("Error: Falla en fork del fantasma. Strerr: " + toString(strerror(errno)), info);
		raise (SIGINT);
	}

	if (pidFantasma == 0) {
		res = execl("Fantasma", "Fantasma", (char*) 0);
		if (res != RES_OK){
			logger->log("Error: Falla en exec del fantasma. Strerr: " + toString(strerror(errno)), info);
			raise (SIGINT);
		}
	}
	logger->log("Se lanzó el niño fantasma", info);

	//todo cambiar por waitpid de calesita
	logger->log("Espero a que muera la calesita", info);
	res = wait(&status);
	if (res == -1){
		logger->log("Error: en el wait de calesita. Strerr: " + toString(strerror(errno)), info);
		raise (SIGINT);
	}

	logger->log("Espero a que muera el recaudador y el admin", info);
	//espera al recaudador y admin todo waitpid
	wait(&status);
	if (res == -1){
		logger->log("Error: en el wait del administrador o recaudador. Strerr: " + toString(strerror(errno)), info);
		raise (SIGINT);
	}
	wait(&status);
	if (res == -1){
		logger->log("Error: en el wait del administrador o recaudador. Strerr: " + toString(strerror(errno)), info);
		raise (SIGINT);
	}

	// Espera al fantasma
	logger->log("Espero a que muera el fantasma", info);
	wait(&status);
	if (res == -1){
		logger->log("Error: en el wait del fantasma. Strerr: " + toString(strerror(errno)), info);
		raise (SIGINT);
	}

	logger->log("Ok ahora mato semaforos", info);
	//mato semaforos
	res = semCalGira->eliminar();
	if ( controlErrores1(res, logger, info) == MUERTE_POR_ERROR ) { raise (SIGINT);}
	delete semCalGira;
	res = semMutexEntrada->eliminar();
	if ( controlErrores1(res, logger, info) == MUERTE_POR_ERROR ) { raise (SIGINT);}
	delete semMutexEntrada;
	res = semCalLug->eliminar();
	if ( controlErrores1(res, logger, info) == MUERTE_POR_ERROR ) { raise (SIGINT);}
	delete semCalLug;
	res = semColaCal->eliminar();
	if ( controlErrores1(res, logger, info) == MUERTE_POR_ERROR ) { raise (SIGINT);}
	delete semColaCal;

	//Si le llega SIGINT llega hasta aca, controlo si llego la señal
	if (sigint_handler->getGracefulQuit() == CERRAR) {
		logger->log("MUERTE POR SIGINT", info);

		//Limpio cosas
		//borro archivos de fifos
		string patron = "cola*";
		/* todo, gran todo
		vector<string> colas = glob(patron);

		vector<string>::iterator it;
		for ( it = colas.begin(); it != colas.end(); it++){

			cout << (*it) << endl;

			//remover arch
			string  com = "rm " + (*it);
			system(com.c_str());

		}
		 */
		//restituyo handler original y me suicido por SIGINT
		SignalHandler::destruir ();

		//todo meter o a sig handler.h o algo
		struct sigaction sa;
		memset(&sa, 0, sizeof(sa));
		sa.sa_handler = SIG_DFL;
		sigaction ( SIGINT,&sa,0 );	// cambiar accion de la senial

		raise(SIGINT);
	}

	logger->log("Terminó la simulación", info);
	logger->end();

	SignalHandler::destruir();
	delete sigint_handler;
}

Lanzador::~Lanzador() {
	delete info;
	delete logger;
}
