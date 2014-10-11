/*
 * Logger.cpp
 *
 *  Created on: 17/9/2014
 *      Author: nicolas
 */

#include "Logger.hpp"
#include <string>
#include <sstream>
#include <ctime>

Logger::Logger() {
	canLog = false;
	output = NULL;
	lockLog = new LockWrite(ARCH_LOCK_LOG);
	setOutput("LogCalesita.log");
	init();
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
void Logger::log(string message, const Info* info) const{

	this->lockLog->tomarLock();

	LogMessage* m = new LogMessage(message, info);
	if (output != 0 && canLog) {
		output->log(m);
	}
	delete m;

	this->lockLog->liberarLock();
}

Logger::~Logger() {
	canLog = false;
	if (output != NULL) {
		delete output;
		output = NULL;
	}
	if (lockLog != NULL) {
		delete lockLog;
		lockLog = NULL;
	}
}

string calculateTime() {
	time_t rawtime;
	struct tm* timeinfo;
	time(&rawtime);
	timeinfo = localtime(&rawtime);
	char buffer[50];
	strftime(buffer, 50, "%Y-%m-%d", timeinfo);
	return string(buffer);
}

void Logger::start() {
	string message =
			"***************************************************************************************************\n"
			"*                                    ConcuCalesita - " + calculateTime() + "                                   *\n"
			"***************************************************************************************************";
	log(message, NULL);
}

void Logger::end() {
	string message = "***************************************************************************************************";
	log(message, NULL);
}
