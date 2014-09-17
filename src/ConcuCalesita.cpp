//============================================================================
// Name        : ConcuCalesita.cpp
// Author      : 
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================


//A ver si puedo cmmitear algo
#include <iostream>
using namespace std;

#include "logger/Logger.hpp"

int main() {

	Logger* log = new Logger();
	log->setLevel(0);
	log->setOutput(new Output("test.log"));

	log->log("Mensaje prueba 1", 0);
	log->log("Mensaje prueba 2", 0);

	delete log;

	return 0;
}
