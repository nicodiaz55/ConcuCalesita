/*
 * Output.cpp
 *
 *  Created on: 16/9/2014
 *      Author: nicolas
 */

#include "Output.hpp"
#include <iostream>
#include <cerrno>
#include <unistd.h>
#include <fcntl.h>

using namespace std;

Output::Output(string output) {
	this->output = output;
}

Output::~Output() {}

/**
 * Loggea un mensaje en la salida.
*/
void Output::log(LogMessage* message) {
	ofstream file(output.c_str(), ios::app | ios::out);
	if (!file.is_open()) {
		cout << "No se pudo escribir en el archivo!!!!!" << endl;
		return;
	}
	string m = message->toString();
	file << m.c_str();
	// tiro por cout tambien:
	cout << m;
	file.close();
}
