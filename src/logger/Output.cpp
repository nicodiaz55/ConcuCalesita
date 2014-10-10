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
	this->file = 0;

}

Output::~Output() {
	if (file != 0) {
		close(file);
		file = 0;
	}

}

/**
 * Inicializa la salida (la crea, la deja lista para escribir)
 */
bool Output::init() {
	file = open(output.c_str(), O_CREAT | O_WRONLY, 0777);

	if (file == -1) {
		// string err = "No se pudo abrir el archivo: " + string(strerror(errno));
		// throw err;
		file = 0;
		return false;
	}

	return true;
}

/**
 * Loggea un mensaje en la salida.
 *
 * TODO: manejar acceso entre procesos con un... Lock
 */
void Output::log(LogMessage* message) {
	if (lseek(file,0,SEEK_END) == -1) {
		// string err = "No se pudo escribir en el archivo: " + string(strerror(errno));
		// throw err;
		return;
	}
	string m = message->toString();
	const void* buffer = m.c_str();
	const ssize_t buffer_size = m.size();

	write(file, buffer, buffer_size);

}
