/*
 * Puerta.cpp
 *
 *  Created on: Sep 30, 2014
 *      Author: juan
 */


#ifdef FILA1

#include "Seniales/SignalHandler.h"
#include "Seniales/SIGUSR1_Handler.h"
#include <memory.h>
#include "Pipes_y_Fifos/FifoEscritura.h"
#include "Pipes_y_Fifos/Pipe.h"

#include "logger/Logger.hpp"
#include "utils/Utils.hpp"


 /*
  * Controla las colas de chicos con pipes
  */

using namespace std;


//todo Control de errores!!


int main ( int argc, char** argv){

	//pongo el manejador de la señal
	SIGUSR1_Handler sigusr1_handler;
	SignalHandler :: getInstance()->registrarHandler ( SIGUSR1,&sigusr1_handler );

	//recibe pipes
	int fdReadKid,fdWriteKid,fdReadPuerta,fdWritePuerta;

	stringstream ss;
	ss.str("");
	ss.clear();
	ss << argv[1];
	ss >> fdReadKid;

	ss.str("");
	ss.clear();
	ss << argv[2];
	ss >> fdWriteKid;

	ss.str("");
	ss.clear();
	ss << argv[3];
	ss >> fdReadPuerta;

	ss.str("");
	ss.clear();
	ss << argv[4];
	ss >> fdWritePuerta;

	Pipe pipe1(fdReadKid,fdWriteKid);
	pipe1.setearModo(Pipe::LECTURA);

	Pipe pipe2(fdReadPuerta,fdWritePuerta);
	pipe2.setearModo(Pipe::ESCRITURA);


	FifoEscritura fifoRecaudador("FifoRecaudador");
	fifoRecaudador.abrir();


	while (sigusr1_handler.getGracefulQuit() != 1){
cout << "pase 4"<< endl;
		int pidKid;
		pipe1.leer(&pidKid, sizeof(int));
cout << "pase 5"<< endl;
		cout << "Fila bol lee del pipe:" << pidKid << " gracequit = " << sigusr1_handler.getGracefulQuit() << endl;

		string ruta = "Cola" + toString(pidKid);
//todo ver que pasaba si se llena un pipe

//le dice al chico que pase
		FifoEscritura fifoAKid(ruta);
cout << "pase 1"<< endl;
		int pasa = 8;
		fifoAKid.escribir( &pasa, sizeof(int) );

		fifoAKid.cerrar();
		fifoAKid.eliminar();
//le avisa al recaudador que pago un chico
		if (pidKid != -1) { //-1 llega para que muera la puerta con la señal (sino puede quedar bloqueada en el read)
			int pago = 1;
cout << "pase Nooooo"<< endl;
			cout << "Le escribo al rec " << " gracequit = " << sigusr1_handler.getGracefulQuit() << endl;
			fifoRecaudador.escribir( &pago, sizeof(int));
		}
//le mete el niño a la otra fila
		//(si, se que la pasa el -1 tmb. aca, pero por seguridad duplico abajo)
		pipe2.escribir(&pidKid, sizeof(int));
cout << "pase 2"<< endl;

	}
cout << "pase 3"<< endl;
	//para que muera el recaudador
	int pago = 2;
	cout << "Le escribo al rec " << pago << " gracequit = " << sigusr1_handler.getGracefulQuit() << endl;
	fifoRecaudador.escribir( &pago, sizeof(int));

	//Le pasa el -1 para que no bloquee en la muerte
	int msj = -1;
	pipe2.escribir(&msj, sizeof(int));

	fifoRecaudador.cerrar();
	fifoRecaudador.eliminar();

	pipe1.cerrar();
	pipe2.cerrar();

	SignalHandler :: destruir ();

return 0;

}

#endif


