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
	int fdRead;

	stringstream ss;
	ss.str("");
	ss.clear();
	ss << argv[1];
	ss >> fdRead;

	cout << "Soy puerta y leo del: "<< fdRead << endl;


	//todo cambiar a señal
	while (seguir){
		int fdWr;
		cout<<"LEooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo"<<endl;
		read(fdRead, &fdWr, sizeof(int));

		cout << "Puerta leyo: "<< fdWr<< endl;

		string strAux = "pasa";
		write(fdWr, strAux.c_str(), 5);

	}


return 0;

}

#endif


