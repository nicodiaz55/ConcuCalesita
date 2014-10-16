/*
 * Lanzador.h
 *
 *  Created on: 12/10/2014
 *      Author: nicolas
 */

#ifndef LANZADOR_H_
#define LANZADOR_H_

#include "./logger/Logger.hpp"

#include "Seniales/SignalHandler.h"
#include "Seniales/SIGINT_Handler.h"
#include "Semaforos/Semaforo.h"
#include "Pipes_y_Fifos/Pipe.h"
#include "Memoria_Compartida/VectorMemoCompartida.h"
#include <sys/wait.h>

#include <cstdlib>
#include <cerrno>

#include "Caja.h"
#include "utils/Utils.hpp"
#include "utils/Random.hpp"

class Lanzador {
private:
	Logger* logger;
	Info* info;
	int cantNinios;
	int lugaresCalesita;
	int tiempoVuelta;
	int auxLugares;
	int precio;
	std::vector< MemoriaCompartida<bool>* > memLugares;

	SIGINT_Handler* sigint_handler;
	Semaforo* semCalSubir;
	Semaforo* semCalLug;
	Semaforo* semColaCal;
	Semaforo* semCalGira;
	Semaforo* semMutexEntrada;
	Semaforo* semAdminRec;
	Pipe* pipeAKids;
	int fdWrPuerta1;

	pid_t pidCal,pidPuerta1,pidPuerta2,pidFantasma,pidAdmin,pidRec;

public:
	Lanzador(int cantNinios, int lugaresCalesita, int tiempoVuelta, int precio);
	void iniciar();
	void lanzarAdministradorYRecaudador();
	void lanzarCalesita();
	void lanzarFilasYNinios();
	void terminar();
	~Lanzador();
};

#endif /* LANZADOR_H_ */
