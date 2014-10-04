/* Recaudador.cpp
 *
 *  Created on: Sep 30, 2014
 *      Author: juan
 */

#ifdef RECAUDADOR


#include "Memoria_Compartida/MemoriaCompartida.h"
#include "Pipes_y_Fifos/FifoLectura.h"
#include "Locks/LockWrite.hpp"
#include "Locks/LockRead.hpp"

#include "logger/Logger.hpp"
#include "utils/Utils.hpp"


 /*
  * Registra en la caja el cobro de boletos
  */

using namespace std;


//todo Control de errores!!


//todo cambiar a pasado por parametro
static const int precio = 2;

int main ( int argc, char** argv){

	FifoLectura fifo("FifoRecaudador");
	fifo.abrir();


	//pide memoria comp. para caja
	MemoriaCompartida<int> caja;
	caja.crear("arch",44);

	//prepara lock de caja recaudacion
	LockFile* lockW = new LockWrite("archLockCaja");

	while (true){

		int avisoPago;
		fifo.leer(&avisoPago , sizeof(int));

		if (avisoPago == 2) { break; }

		lockW->tomarLock();

		caja.escribir (caja.leer() + precio);

		lockW->liberarLock();

	}

	//para que muera el administrador
	lockW->tomarLock();
	caja.escribir(-1);
	lockW->liberarLock();

	//libero memoria compartida
 	caja.liberar();

 	delete lockW;

	fifo.cerrar();
	fifo.eliminar();


return 0;

}

#endif
