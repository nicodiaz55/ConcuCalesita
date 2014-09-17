/*
 * Logger.cpp
 *
 *  Created on: 17/9/2014
 *      Author: nicolas
 */

#include "Logger.hpp"

Logger::Logger() {
	level = -1;
	output = 0;
}

void Logger::setLevel(int level) {
	this->level = level;
}

void Logger::setOutput(Output* output) {
	if (output->init())
		this->output = output;
}

void Logger::log(string message, int level) {
	LogMessage* m = createLogMessage(message, level);
	if (isPublishable(m) && output) {
		output->log(m);
	}
	delete m;
}

Logger::~Logger() {
	level = -1;
	if (output) {
		delete output;
		output = 0;
	}
}

LogMessage* Logger::createLogMessage(string message, int level) {
	return new LogMessage(message, level);
}

bool Logger::isPublishable(LogMessage* message) {
	return message->getLevel() >= level; // TODO: revisar desigualdad
}
