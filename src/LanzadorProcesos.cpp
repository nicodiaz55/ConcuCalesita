/*
 * LanzadorProcesos.cpp
 *
 *  Created on: Sep 26, 2014
 *      Author: juan
 */

#ifdef LANZADOR

#include "Seniales/SignalHandler.h"
#include "Seniales/SIGINT_Handler.h"
#include "Semaforos/Semaforo.h"
#include "Pipes_y_Fifos/Pipe.h"
#include <sys/wait.h>

#include <stdlib.h>
#include <errno.h>

#include "Constantes.h"
#include "Caja.h"
#include "logger/Logger.hpp"
#include "utils/Utils.hpp"
#include "utils/Random.hpp"


/*
 * Lanza chicos (Procesos Kid) con fork y exec
 * Lanza calesita, administrador y recaudador
 *
 * Inicializa semaforos y shMem antes de que haya otros procesos para prevenir competencias.
 */

//todo Limpiar includes de cosas innecesarias

using namespace std;

//todo Control de errores!!

int leerParametros(const int argc, char** argv, int& cantNinios,int& lugaresCalesita,int& tiempoVuelta,const Logger* logger, const Info* info){

	if (argc < 4) {
		cout
		<< "Cantidad de parametros incorrectos, especifique numero de niños, cantidad de lugares y tiempo de vuelta"
		<< endl
		<< "cantidad de niños : -n"	 << endl
		<< "cantidad de lugares : -l"	 << endl
		<< "tiempo de vuelta : -t"	 << endl;
		return RES_PARAM_NUM_ERR;
	}

	int option;

	while ((option = getopt(argc, argv, "l:n:t:")) != -1) {
		switch (option) {

		case 't':
			tiempoVuelta = toInt(optarg);
			logger->log("El tiempo de vuelta es de: " + toString(tiempoVuelta)	+ " segundos", info);
			break;

		case 'l':
			lugaresCalesita = toInt(optarg);
			logger->log("La cantidad de lugares es: " + toString(lugaresCalesita),	info);
			break;

		case 'n':
			cantNinios = toInt(optarg);
			logger->log("La cantidad de niños en el barrio es: " + toString(cantNinios), info);
			break;

		default:
			logger->log("Se ingresó una opción inválida.", info);
			return RES_PARAM_INV;
			break;

		}
	}

	return RES_OK;
}

int main ( int argc, char** argv){

	//Abro el logger
	Logger* logger = new Logger();
	Info* info = new Info(getpid(), "Lanzador");

	logger->start();

	logger->log("Empieza la simulación", info);

	//pongo el manejador de la señal
	SIGINT_Handler sigint_handler;
	int res = SignalHandler :: getInstance()->registrarHandler(SIGINT, &sigint_handler);
	if (res != RES_OK) {
		logger->log("Error: " + toString(res) + ". Strerr: " + toString(strerror(errno)),info);
		raise(SIGINT);
	}


	int cantNinios = 0;
	int lugaresCalesita = 0;
	int tiempoVuelta = 0;

	//LECTURA DE PARAMETROS
	res = leerParametros(argc, argv, cantNinios, lugaresCalesita, tiempoVuelta, logger, info);
	if (res != RES_OK){
		logger->log("Error: " + toString(res), info);
		raise(SIGINT);
	}

	//INICIALIZAR IPC mechanisms

	//INICIALIZA SEMAFOROS
	Semaforo semCalGira("/etc", 22, 0); //indica si la calesita esta girando, 0 esta girando, >0 cant gente que puede bajar
	res = semCalGira.crear();
	if ( controlErrores1(res, logger, info) == MUERTE_POR_ERROR ) { raise (SIGINT);}

	Semaforo semMutexEntrada("/etc", 24, 1); //indica si se puede entrar o no al parque (para que los niños "entren y salgan de a uno")
	res = semMutexEntrada.crear();
	if ( controlErrores1(res, logger, info) == MUERTE_POR_ERROR ) { raise (SIGINT);}

	int auxLugares = 0;

	//si los chicos no alcanzan a llenarla solo espera a que se sienten todos para arrancar
	if (cantNinios < lugaresCalesita) {
		auxLugares = cantNinios;
	} else {
		auxLugares = lugaresCalesita;
	}

	Semaforo semCalLug("/etc", 23, auxLugares); //indica la cantidad de lugares disponibles en calesita
	Semaforo semColaCal("/etc", 25, auxLugares); //indica cuantos niños pueden pasar la cola para subir a la calesita

	res = semCalLug.crear();
	if ( controlErrores1(res, logger, info) == MUERTE_POR_ERROR ) { raise (SIGINT);}

	res = semColaCal.crear();
	if ( controlErrores1(res, logger, info) == MUERTE_POR_ERROR ) { raise (SIGINT);}

	//INICIALIZA MEMORIA COMPARTIDA
	MemoriaCompartida<int> kidsInPark;
	res = kidsInPark.crear("/etc", 33, PERMISOS_USER_RDWR);
	if ( controlErrores1(res, logger, info) == MUERTE_POR_ERROR ) { raise (SIGINT);}

	//Memoria compartida de cantidad de chicos en parque
	kidsInPark.escribir(0);

	//Memoria compartida para la caja
	Caja caja;
	res = caja.init();
	caja.iniciarRecaudacion();
	if ( controlErrores1(res, logger, info) == MUERTE_POR_ERROR ) { raise (SIGINT);}

	//Memoria compartida para la calesita
	MemoriaCompartida<int> continua;
	res = continua.crear("/etc", 55, PERMISOS_USER_RDWR);
	if ( controlErrores1(res, logger, info) == MUERTE_POR_ERROR ) { raise (SIGINT);}

	continua.escribir(0);

	//LANZAR RECAUDADOR Y ADMINISTRADOR
	pid_t pidRec = fork();

	if (pidRec == -1){
		logger->log("Error: Falla fork de recaudador. Strerr: " + toString(strerror(errno)), info);
		raise (SIGINT);
	}

	if (pidRec == 0) {
		res = execl("Recaudador", "Recaudador", (char*) 0);
		if (res != RES_OK){
			logger->log("Error: Falla en exec del recaudador. Strerr: " + toString(strerror(errno)), info);
			raise (SIGINT);
		}
	}

	logger->log("Se lanzó el recaudador", info);

	pid_t pidAdmin = fork();

	if (pidAdmin == -1){
		logger->log("Error: Falla en fork del administrador. Strerr: " + toString(strerror(errno)), info);
		raise (SIGINT);
	}

	if (pidAdmin == 0) {
		res = execl("Administrador", "Administrador", (char*) 0);
		if (res != RES_OK){
			logger->log("Error: Falla en exec del administrador. Strerr: " + toString(strerror(errno)), info);
			raise (SIGINT);
		}
	}

	logger->log("Se lanzó el administrador", info);
	//desattachea de la caja una vez que la tienen Recaud. y Admin.
	res = caja.terminar();
	if ( controlErrores1(res, logger, info) == MUERTE_POR_ERROR ) { raise (SIGINT);}


	//LANZAR CALESITA

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
	if ( controlErrores1(res, logger, info) == MUERTE_POR_ERROR ) { raise (SIGINT);}	//LANZAR NIÑOS Y PUERTAS

	//crea los pipes que van de los niños a las puerta y entre puertas y lanza puertas

	Pipe pipeEntrePuertas; // entre la fila de boletos y la de la calesita
	res = pipeEntrePuertas.crear();
	if ( controlErrores1(res, logger, info) == MUERTE_POR_ERROR ) { raise (SIGINT);}

	int fdRdPuerta2 = pipeEntrePuertas.getFdLectura(); // puerta de la calesita
	int fdWrPuerta2 = pipeEntrePuertas.getFdEscritura(); //para escribirle a la puerta 2

	pid_t pidPuerta2 = fork();
	if (pidPuerta2 == -1){
		logger->log("Error: Falla en fork de la fila de la calesita. Strerr: " + toString(strerror(errno)), info);
		raise (SIGINT);
	}

	if (pidPuerta2 == 0) {
		res = execl("FilaCalesita", "Puerta2", toString(fdRdPuerta2).c_str(),
				toString(fdWrPuerta2).c_str(), (char*) 0);
		if (res != RES_OK){
			logger->log("Error: Falla en exec de la fila de la calesita. Strerr: " + toString(strerror(errno)), info);
			raise (SIGINT);
		}
	}

	logger->log("Se lanzó la fila para subir a la calesita", info);

	//pipe para que los ninios se encolen para boleto
	Pipe pipeAKids;
	res = pipeAKids.crear();
	if ( controlErrores1(res, logger, info) == MUERTE_POR_ERROR ) { raise (SIGINT);}

	int fdRdPuerta1 = pipeAKids.getFdLectura();
	int fdWrPuerta1 = pipeAKids.getFdEscritura(); //para escribirle a la puerta 1

	pid_t pidPuerta1 = fork();
	if (pidPuerta1 == -1){
		logger->log("Error: Falla en fork de la fila de boletos. Strerr: " + toString(strerror(errno)), info);
		raise (SIGINT);
	}

	if (pidPuerta1 == 0) {
		res = execl("FilaBoleto", "Fila1", toString(fdRdPuerta1).c_str(),
				toString(fdWrPuerta1).c_str(), toString(fdRdPuerta2).c_str(),
				toString(fdWrPuerta2).c_str(), (char*) 0);
		if (res != RES_OK){
			logger->log("Error: Falla en exec de la fila de boletos. Strerr: " + toString(strerror(errno)), info);
			raise (SIGINT);
		}
	}

	logger->log("Se lanzó la fila para comprar boleto", info);

	pipeEntrePuertas.cerrar();

	//lanza niños
	for (int i = 0; i < cantNinios; i++) {

		pid_t pid = fork();
		if (pid == -1){
			logger->log("Error: Falla en fork de un chico. Strerr: " + toString(strerror(errno)), info);
			raise (SIGINT);
		}

		if (pid == 0) {
			res = execl("Kid", "Kid", toString(fdRdPuerta1).c_str(),
					toString(fdWrPuerta1).c_str(), (char*) 0);
			if (res != RES_OK){
				logger->log("Error: Falla en exec de un chico. Strerr: " + toString(strerror(errno)), info);
				raise (SIGINT);
			}
		}

		logger->log("Se lanzó un niño", info);
		usleep((int) exponentialTime(2));
	}

	//se desattachea esta shared mem. despues de crear los hijos, asi siempre hay alguien usandola.
	res = kidsInPark.liberar();
	if ( controlErrores1(res, logger, info) == MUERTE_POR_ERROR ) { raise (SIGINT);}


	//COMIENZO DEL FIN
	//espero que terminen todos los hijos
	int status;
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
	pipeAKids.cerrar();

	res = wait(&status); //todo waitpid
	if (res == -1){
		logger->log("Error: en el wait puerta 1. Strerr: " + toString(strerror(errno)), info);
		raise (SIGINT);
	}

	//para destrabar la puerta 2
	res = semColaCal.v(1);
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
	res = semCalGira.eliminar();
	if ( controlErrores1(res, logger, info) == MUERTE_POR_ERROR ) { raise (SIGINT);}

	res = semMutexEntrada.eliminar();
	if ( controlErrores1(res, logger, info) == MUERTE_POR_ERROR ) { raise (SIGINT);}

	res = semCalLug.eliminar();
	if ( controlErrores1(res, logger, info) == MUERTE_POR_ERROR ) { raise (SIGINT);}

	res = semColaCal.eliminar();
	if ( controlErrores1(res, logger, info) == MUERTE_POR_ERROR ) { raise (SIGINT);}
	//Si le llega SIGINT llega hasta aca, controlo si llego la señal
	if (sigint_handler.getGracefulQuit() == CERRAR){

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
		SignalHandler :: destruir ();

		//todo meter o a sig handler.h o algo
		struct sigaction sa;
		memset(&sa, 0, sizeof(sa));
		sa.sa_handler = SIG_DFL;
		sigaction ( SIGINT,&sa,0 );	// cambiar accion de la senial

		raise(SIGINT);
	}

	logger->log("Terminó la simulación", info);

	logger->end();

	SignalHandler :: destruir ();

	if (logger != NULL) {
		delete logger;
		logger = NULL;
	}

	if (info != NULL) {
		delete info;
		info = NULL;
	}

	return RES_OK;

}

#endif


