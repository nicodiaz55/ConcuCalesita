/*
 * Caja.cpp
 *
 *  Created on: 11/10/2014
 *      Author: nicolas
 */

#include "Caja.h"
#include <iostream>
using namespace std;

Caja::Caja() {

}

int Caja::init() {
	int res = recaudado.crear("/etc", 44, PERMISOS_USER_RDWR);
	int res2 = estado.crear("/etc", 10000, PERMISOS_USER_RDWR);

	if (res != RES_OK ){
		return res;
	}
	if (res2 != RES_OK) {
		return res2;
	}

	cout << "recaudado: " << recaudado.shmId << endl;
	cout << "estado: " << estado.shmId << endl;
	estado.escribir(true);

	return RES_OK;
}

bool Caja::obtenerEstado() {
	return estado.leer();
}

void Caja::setearEstado(bool estado) {
	this->estado.escribir(estado);
}

int Caja::obtenerRecaudacion() {
	return recaudado.leer();
}

int Caja::terminar() {

	int res = recaudado.liberar();
	if (res == RES_OK){
		return res;
	}

	res =  estado.liberar();
	if (res == RES_OK){
		return res;
	}

	return res;
}

void Caja::iniciarRecaudacion() {
	recaudado.escribir(0);
}

int Caja::aumentarRecaudacion(int valor) {
	if (valor < 0)
		return valor;
	if (estado.leer() == false) {
		return -1;
	}
	recaudado.escribir(recaudado.leer() + valor);
	return 0;
}


Caja::~Caja() {

}


