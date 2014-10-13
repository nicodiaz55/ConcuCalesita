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

#include "Constantes.h"
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
	std::vector< MemoriaCompartida<bool>* > memLugares;

	SIGINT_Handler* sigint_handler;
	Semaforo* semCalLug;
	Semaforo* semColaCal;
	Semaforo* semCalGira;
	Semaforo* semMutexEntrada;
	Pipe* pipeAKids;
	int fdWrPuerta1;

public:
	Lanzador(int cantNinios, int lugaresCalesita, int tiempoVuelta);
	void iniciar();
	void lanzarAdministradorYRecaudador();
	void lanzarCalesita();
	void lanzarFilasYNinios();
	//void esperarFin();
	void terminar();
	~Lanzador();
};

#endif /* LANZADOR_H_ */
