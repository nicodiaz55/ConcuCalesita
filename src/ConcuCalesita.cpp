/*
 * ConcuCalesita.cpp
 *
 *  Created on: 12/10/2014
 *      Author: nicolas
 */

#ifdef MAIN

#include "Lanzador.h"
#include <cstdlib>
#include <cerrno>

using namespace std;

int leerParametros(const int argc, char** argv, int& cantNinios, int& lugaresCalesita, int& tiempoVuelta, int& precio) {
	if (argc < 5) {
		cout
		<< "Cantidad de parametros incorrectos, especifique numero de niños, cantidad de lugares, tiempo de vuelta y precio"
		<< endl
		<< "cantidad de niños : -n"	 << endl
		<< "cantidad de lugares : -l"	 << endl
		<< "tiempo de vuelta : -t"	 << endl
		<< "precio de la vuelta: -p"	 << endl;

		return RES_PARAM_NUM_ERR;
	}

	int option;
	while ((option = getopt(argc, argv, "l:n:t:p:")) != -1) {
		switch (option) {
			case 't':
				tiempoVuelta = toInt(optarg);
				break;

			case 'l':
				lugaresCalesita = toInt(optarg);
				break;

			case 'n':
				cantNinios = toInt(optarg);
				break;

			case 'p':
				precio = toInt(optarg);
				break;
			default:
				cout << "Se ingresó una opción inválida." << endl;
				return RES_PARAM_INV;
				break;
		}
	}
	return RES_OK;
}

int main ( int argc, char** argv) {
	int cantNinios = 0;
	int lugaresCalesita = 0;
	int tiempoVuelta = 0;
	int precio = 0;
	int res = leerParametros(argc, argv, cantNinios, lugaresCalesita, tiempoVuelta, precio);
	if (res != RES_OK) {
		cout << "Error: " + toString(res) << endl;
		raise(SIGINT);
	}
	Lanzador lanzador(cantNinios, lugaresCalesita, tiempoVuelta, precio);
	lanzador.iniciar();
	lanzador.lanzarAdministradorYRecaudador();
	lanzador.lanzarCalesita();
	lanzador.lanzarFilasYNinios();

	lanzador.terminar();

	return RES_OK;
}

#endif
