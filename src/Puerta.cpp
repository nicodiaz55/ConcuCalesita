/*
 * Puerta.cpp
 *
 *  Created on: Sep 30, 2014
 *      Author: juan
 */


#ifdef PUERTA

#include<iostream>
#include <stdio.h>

#include <sys/types.h>
#include <sys/ipc.h>
#include <signal.h>
#include <memory.h>

#include <unistd.h>
#include <stdlib.h>

#include "logger/Logger.hpp"
#include "utils/Utils.hpp"
 /*
  * Controla las colas de chicos con pipes
  */

using namespace std;


//todo Control de errores!!

bool seguir = true;

void manejaSig(int sig){
	if(sig == SIGUSR1){
		cout<< "MUERO" << endl;
		seguir = false;
	}
}

int main ( int argc, char** argv){

	//pongo el manejador de la señal
	struct sigaction sa;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	sa.sa_handler = manejaSig;
	sigaction(SIGUSR1,&sa,0);

	//recibe pipes
	int fdRead, fdWrRecaudador;

	stringstream ss;
	ss.str("");
	ss.clear();
	ss << argv[1];
	ss >> fdRead;

	cout << "Soy puerta y leo del: "<< fdRead << endl;

	ss;
	ss.str("");
	ss.clear();
	ss << argv[2];
	ss >> fdWrRecaudador;

	cout << "Puerta le escribe al: "<< fdWrRecaudador << endl;

	while (seguir){
		int fdWr;
		cout<<"LEooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo"<<endl;
		read(fdRead, &fdWr, sizeof(int));
//todo ver que pasaba si se llena un pipe
		cout << "Puerta leyo: "<< fdWr<< endl;
//le dice al chico que pase
		string strAux = "pasa";
		write(fdWr, strAux.c_str(), 5);
//le avisa al recaudador que pago un chico
		if (fdWr != -1) { //-1 llega para que muera la puerta con la señal (sino queda bloqueada en el read)
			int pago = 1;
			write(fdWrRecaudador, &pago, sizeof(int));
		}

	}
	//para que muera el recaudador
	int pago = 2;
	write(fdWrRecaudador, &pago, sizeof(int));


return 0;

}

#endif


