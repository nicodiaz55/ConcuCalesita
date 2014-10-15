/*
 * Lanzador.cpp
 *
 *  Created on: 12/10/2014
 *      Author: nicolas
 */

#include "Lanzador.h"

using namespace std;

Lanzador::Lanzador(int cantNinios, int lugaresCalesita, int tiempoVuelta, int precio, int tiempoAdmin, int mediaKid) {
	logger = new Logger();
	info = new Info(getpid(), "Lanzador");
	this->cantNinios = cantNinios;
	this->lugaresCalesita = lugaresCalesita;
	this->tiempoVuelta = tiempoVuelta;
	this->precio = precio;
	this->adminTMax = tiempoAdmin;
	this->mediaKid = mediaKid;
	auxLugares = 0;
	sigint_handler = NULL;
	semCalLug = NULL;
	semCalSubir = NULL;
	semColaCal = NULL;
	semCalGira = NULL;
	semMutexEntrada = NULL;
	semAdminRec = NULL;
	fdWrPuerta1 = 0;
	pipeAKids = NULL;
	pidCal = 0;
	pidPuerta1 = 0;
	pidPuerta2 = 0;
	pidFantasma = 0;
	pidAdmin = 0;
	pidRec = 0;
	logger->start();
	logger->log("El tiempo de vuelta es de: " + toString(tiempoVuelta)	+ " segundos", info);
	logger->log("La cantidad de lugares es: " + toString(lugaresCalesita),	info);
	logger->log("La cantidad de niños en el barrio es: " + toString(cantNinios), info);
	logger->log("El precio de la vuelta es de: " + toString(precio), info);
	logger->log("El administrador consultara la caja cada Uniforme[1," + toString(adminTMax) + "] seg", info);
	logger->log("Los niños llegan con una distribución Poisson de media " + toString(mediaKid) + " seg", info);
}

void Lanzador::iniciar() {
	logger->log("Empieza la simulación", info);

	//pongo el manejador de la señal
	sigint_handler = new SIGINT_Handler;
	int res = SignalHandler::getInstance()->registrarHandler(SIGINT, sigint_handler);
	if (res != RES_OK) {
		logger->log("Error: " + toString(res) + ". Strerr: " + toString(strerror(errno)),info);
		raise(SIGINT);
	}

	//INICIALIZAR IPC mechanisms

	//INICIALIZA SEMAFOROS
	semCalGira = new Semaforo(ARCH_SEM, SEM_CAL_GIRA, 0); //indica si la calesita esta girando, 0 esta girando, >0 cant gente que puede bajar
	res = semCalGira->crear();
	if (controlErrores1(res, logger, info) == MUERTE_POR_ERROR) {raise(SIGINT);}

	semMutexEntrada = new Semaforo(ARCH_SEM, SEM_MUTEX_ENTR, 1); //indica si se puede entrar o no al parque (para que los niños "entren y salgan de a uno")
	res = semMutexEntrada->crear();
	if (controlErrores1(res, logger, info) == MUERTE_POR_ERROR) {raise(SIGINT);}

	auxLugares = 0;

	//si los chicos no alcanzan a llenarla solo espera a que se sienten todos para arrancar
	if (cantNinios < lugaresCalesita) {
		auxLugares = cantNinios;
	} else {
		auxLugares = lugaresCalesita;
	}

	semCalLug = new Semaforo(ARCH_SEM, SEM_CAL_LUG, auxLugares); //indica la cantidad de lugares disponibles en calesita
	semColaCal = new Semaforo(ARCH_SEM, SEM_COLA_CAL, auxLugares); //indica cuantos niños pueden pasar la cola para subir a la calesita
	semCalSubir =new Semaforo(ARCH_SEM, SEM_CAL_SUBIR, auxLugares); //indica cuantos niños pueden subir a la calesita

	res = semCalSubir->crear();
	if (controlErrores1(res, logger, info) == MUERTE_POR_ERROR) {raise(SIGINT);}

	res = semCalLug->crear();
	if (controlErrores1(res, logger, info) == MUERTE_POR_ERROR) {raise(SIGINT);}

	res = semColaCal->crear();
	if (controlErrores1(res, logger, info) == MUERTE_POR_ERROR) {raise(SIGINT);}

	semAdminRec = new Semaforo(ARCH_SEM, SEM_ADMIN_REC, 0); //para sincronizar con el administrador (caso especial 0 chicos)
	res = semAdminRec->crear();
}

void Lanzador::lanzarAdministradorYRecaudador() {
	//Memoria compartida para la caja
	Caja caja;
	int res = caja.init();
	caja.iniciarRecaudacion();
	if (controlErrores1(res, logger, info) == MUERTE_POR_ERROR) {raise(SIGINT);}

	pidAdmin = fork();
	if (pidAdmin == -1) {
		logger->log("Error: Falla en fork del administrador. Strerr: " + toString(strerror(errno)), info);
		raise (SIGINT);
	}
	if (pidAdmin == 0) {
		res = execl("Administrador", "Administrador", toString(adminTMax).c_str(), (char*) 0);
		if (res != RES_OK) {
			logger->log("Error: Falla en exec del administrador. Strerr: " + toString(strerror(errno)), info);
			raise (SIGINT);
		}
	}
	logger->log("Se lanzó el administrador", info);

	pidRec = fork();
	if (pidRec == -1) {
		logger->log("Error: Falla fork de recaudador. Strerr: " + toString(strerror(errno)), info);
		raise (SIGINT);
	}
	if (pidRec == 0) {
		res = execl("Recaudador", "Recaudador", toString(precio).c_str(), (char*) 0);
		if (res != RES_OK) {
			logger->log("Error: Falla en exec del recaudador. Strerr: " + toString(strerror(errno)), info);
			raise (SIGINT);
		}
	}
	logger->log("Se lanzó el recaudador", info);

	//desattachea de la caja una vez que la tienen Recaud. y Admin.
	res = caja.terminar();
	if (controlErrores1(res, logger, info) == MUERTE_POR_ERROR) {raise(SIGINT);}
}

void Lanzador::lanzarCalesita() {
	//Memoria compartida para la calesita
	MemoriaCompartida<int> continua;
	int res = continua.crear(ARCH_SEM, MEM_CONTINUA, PERMISOS_USER_RDWR);
	if (controlErrores1(res, logger, info) == MUERTE_POR_ERROR) {raise(SIGINT);}

	continua.escribir(0);

	string arg1 = toString(auxLugares);
	string arg2 = toString(lugaresCalesita);
	string arg3 = toString(tiempoVuelta);

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

}

void Lanzador::lanzarFilasYNinios() {
	Pipe pipeEntrePuertas; // entre la fila de boletos y la de la calesita
	int res = pipeEntrePuertas.crear();
	if (controlErrores1(res, logger, info) == MUERTE_POR_ERROR) {raise(SIGINT);}

	int fdRdPuerta2 = pipeEntrePuertas.getFdLectura(); // puerta de la calesita
	int fdWrPuerta2 = pipeEntrePuertas.getFdEscritura(); //para escribirle a la puerta 2

	pidPuerta2 = fork();
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

	pidPuerta1 = fork();
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
	res = kidsInPark.crear(ARCH_SEM, MEM_KIDS, PERMISOS_USER_RDWR);
	if (controlErrores1(res, logger, info) == MUERTE_POR_ERROR) {raise(SIGINT);}

	//Memoria compartida de cantidad de chicos en parque
	kidsInPark.escribir(0);

	//Memoria compartida para cada lugar

	VectorMemoCompartida<bool> lugares;

	res = lugares.crear(ARCH_SEM,INICIO_CLAVES_LUGARES,PERMISOS_USER_RDWR,this->lugaresCalesita);
	if ( controlErrores1(res, logger, info) == MUERTE_POR_ERROR ) { raise (SIGINT);}

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
		sleep((int) exponentialTime(mediaKid));
	}

	//se desattachea de estas shared mem. despues de crear los hijos, asi siempre hay alguien usandola.
	if ( controlErrores1(res, logger, info) == MUERTE_POR_ERROR ) { raise (SIGINT);}
	res = kidsInPark.liberar();

	res = lugares.liberar();
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

	logger->log("Todos los niños salieron del parque. Comienzo a terminar la simulación", info);

	//Le dice a las puertas que mueran
	//espera a las puertas
	logger->log("Informo a la fila que debe terminar...", info);
	int aux = CERRAR_FILA;
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
	logger->log("Cerrando puertas...", info);
	res = waitpid(pidPuerta1, &status, 0);
	if (res == -1){
		logger->log("Error: en el wait puerta 1. Strerr: " + toString(strerror(errno)), info);
		raise (SIGINT);
	}

	//para destrabar la puerta 2
	res = semColaCal->v(1);
	if ( controlErrores1(res, logger, info) == MUERTE_POR_ERROR ) { raise (SIGINT);}

	res = waitpid(pidPuerta2, &status, 0);
	if (res == -1){
		logger->log("Error: en el wait puerta 2. Strerr: " + toString(strerror(errno)), info);
		raise (SIGINT);
	}
	//espera a la calesita
	//puede que la calesita quede bloqueada esperando a un ultimo niño-> mando un fantasma

	//hace que un "chico fantasma" de la ultima vuelta
	pidFantasma = fork();
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


	logger->log("Espero a que muera la calesita [" + toString(pidCal) + "]", info);
	res = waitpid(pidCal, &status, 0);
	if (res == -1){
		logger->log("Error: en el wait de la calesita. Strerr: " + toString(strerror(errno)), info);
		raise (SIGINT);
	}

	// Espera al fantasma
	logger->log("Espero a que muera el fantasma [" + toString(pidFantasma) + "]", info);
	waitpid(pidFantasma, &status, 0);
	if (res == -1){
		logger->log("Error: en el wait del fantasma. Strerr: " + toString(strerror(errno)), info);
		raise (SIGINT);
	}

	logger->log("Espero a que muera el recaudador [" + toString(pidRec) + "]", info);
	waitpid(pidRec, &status, 0);
	if (res == -1){
		logger->log("Error: en el wait del recaudador. Strerr: " + toString(strerror(errno)), info);
		raise (SIGINT);
	}
	logger->log("Espero a que muera el administrador [" + toString(pidAdmin) + "]", info);
	waitpid(pidAdmin, &status, 0);
	if (res == -1){
		logger->log("Error: en el wait del administrador. Strerr: " + toString(strerror(errno)), info);
		raise (SIGINT);
	}

	logger->log("Libero recursos", info);
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
	res = semAdminRec->eliminar();
	if ( controlErrores1(res, logger, info) == MUERTE_POR_ERROR ) { raise (SIGINT);}
	delete semAdminRec;
	res = semCalSubir->eliminar();
	if ( controlErrores1(res, logger, info) == MUERTE_POR_ERROR ) { raise (SIGINT);}
	delete semCalSubir;
	//Si le llega SIGINT llega hasta aca, controlo si llego la señal
	if (sigint_handler->getGracefulQuit() == CERRAR) {
		logger->log("MUERTE POR SIGINT", info);
	}else{
		logger->log("Terminó la simulación", info);
	}

	logger->end();
	SignalHandler::destruir();
	delete sigint_handler;
}

Lanzador::~Lanzador() {
	delete info;
	delete logger;
}
