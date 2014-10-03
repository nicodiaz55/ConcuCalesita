* Puerta.cpp
 *
 *  Created on: Sep 30, 2014
 *      Author: juan
 */


#ifdef PUERTA

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
	int fdRead,fdWrite;

	stringstream ss;
	ss.str("");
	ss.clear();
	ss << argv[1];
	ss >> fdRead;

	ss.str("");
	ss.clear();
	ss << argv[2];
	ss >> fdWrite;

	Pipe pipe(fdRead,fdWrite);
	pipe.setearModo(Pipe::LECTURA);


	FifoEscritura fifoRecaudador("FifoRecaudador");
	fifoRecaudador.abrir();


	while (sigusr1_handler.getGracefulQuit() != 1){
		int fdWr;
		read(fdRead, &fdWr, sizeof(int));

		string ruta = "Cola" + toString(fdWr);
//todo ver que pasaba si se llena un pipe

//le dice al chico que pase
		FifoEscritura fifoAKid(ruta);

		string strAux = "pasa";
		fifoAKid.escribir( strAux.c_str(), strAux.length() + 1 );

		fifoAKid.cerrar();
		fifoAKid.eliminar();
//le avisa al recaudador que pago un chico
		if (fdWr != -1) { //-1 llega para que muera la puerta con la señal (sino queda bloqueada en el read)
			int pago = 1;
			fifoRecaudador.escribir( &pago, sizeof(int));
		}

	}
	//para que muera el recaudador
	int pago = 2;
	fifoRecaudador.escribir( &pago, sizeof(int));


	fifoRecaudador.cerrar();
	fifoRecaudador.eliminar();

	pipe.cerrar();

	SignalHandler :: destruir ();

return 0;

}

#endif
