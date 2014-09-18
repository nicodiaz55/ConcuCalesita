//============================================================================
// Name        : ConcuCalesita.cpp
// Author      : 
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

using namespace std;

#include <unistd.h>
#include "logger/Logger.hpp"
#include "logger/Info.hpp"

int main() {

	/* Todos los procesos van a tener que ejecutar esta linea, por ejemplo, cuando se crean.
	 * Deben mantenerlas en su memoria para pasarselas al log, asi esta sabe a quien esta loggeando.
	 * Se le podría guardar más cosas a esta clase Info. Por el momento, solo el PID y el nombre del proceso.
	 */

	Info* myInfo = new Info(getpid(), "Main");

	Logger* log = new Logger();
	log->setOutput("test.log"); // Se define un archivo de salida
	log->init(); // Siempre se inicializa LUEGO de setear el output.
	// No se va a inicializar si no se seteo el output (o falló)

	log->log("Mensaje prueba 1", myInfo);
	log->log("Mensaje prueba 2", myInfo);

	log->stop(); // No es necesario en el ejemplo, pero why not?

	log->log("Esto no deberia loggearse", myInfo);

	delete log; // El que cree el log, debera borrarlo al final. Un proceso solo creará el log, el "principal"
	delete myInfo; // Cada proceso va a tener que borrar su Info cuando muera.

	return 0;
}
