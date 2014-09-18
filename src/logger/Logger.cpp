/*
 * Logger.cpp
 *
 *  Created on: 17/9/2014
 *      Author: nicolas
 */

#include "Logger.hpp"

Logger::Logger() {
	canLog = false;
	output = 0;
}

void Logger::init() {
	if (output != 0)
		this->canLog = true;
}

void Logger::stop() {
	this->canLog = false;
}

/**
 * Especifica la salida donde se loggea
 */
void Logger::setOutput(string output) {
	this->output = new Output(output);
	if (!this->output->init()) {
		this->output = 0;
		canLog = false;
	}
}

/**
 * Loggea un mensaje
 */
void Logger::log(string message, Info* info) {
	LogMessage* m = new LogMessage(message, info);
	if (output != 0 && canLog) {
		output->log(m);
	}
	delete m;
}

Logger::~Logger() {
	canLog = false;
	if (output) {
		delete output;
		output = 0;
	}
}
