/*
 * Caja.h
 *
 *  Created on: 11/10/2014
 *      Author: nicolas
 */

#ifndef CAJA_H_
#define CAJA_H_

#include "Memoria_Compartida/MemoriaCompartida.h"

class Caja {
private:
	MemoriaCompartida<int> recaudado;
	MemoriaCompartida<bool> estado;
public:
	Caja();
	int init();
	bool obtenerEstado();
	void setearEstado(bool estado);
	int obtenerRecaudacion();
	void iniciarRecaudacion();
	int aumentarRecaudacion(int valor);
	int terminar();
	~Caja();
};

#endif /* CAJA_H_ */
