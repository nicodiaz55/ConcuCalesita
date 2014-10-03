/*
 * Administrador.cpp
 *
 *  Created on: Oct 1, 2014
 *      Author: juan
 */

#ifdef ADMIN

#include "Memoria_Compartida/MemoriaCompartida.h"
#include "Locks/LockWrite.hpp"
#include "Locks/LockRead.hpp"

#include "logger/Logger.hpp"
#include "utils/Utils.hpp"


 /*
  * Mira la caja de boletos e imprime por pantalla o al log o whatever
  */

using namespace std;

//todo Control de errores!!



int main ( int argc, char** argv){

	//pide memoria comp. para caja

	MemoriaCompartida<int> caja;
	caja.crear("arch",44); //todo permisos

	//prepara lock de caja recaudacion
	LockFile* lockR = new LockRead("archLockCaja");

	while (true){

		lockR->tomarLock();

		if (caja.leer() >= 0){
			cout<<"Admin: \"En caja hay:\" " << caja.leer() << endl;
		}else{
			break;
		}

		lockR->liberarLock();

		//este sleep esta para que no llene el log solo con sus lecturas
		sleep(1);

	}

	//libero memoria compartida
	caja.liberar();

	delete lockR;


return 0;

}

#endif

