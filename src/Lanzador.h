/*
 * Lanzador.h
 *
 *  Created on: 12/10/2014
 *      Author: nicolas
 */

#ifndef LANZADOR_H_
#define LANZADOR_H_

#include "./logger/Logger.hpp"

class Lanzador {
private:
	Logger* logger;
	Info* info;
	int cantNinios;
	int lugaresCalesita;
	int tiempoVuelta;
	int auxLugares;

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
	void esperarFin();
	void terminar();
	~Lanzador();
};

#endif /* LANZADOR_H_ */
