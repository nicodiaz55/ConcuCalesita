/*
 * LogMessage.cpp
 *
 *  Created on: 16/9/2014
 *      Author: nicolas
 */

#include "LogMessage.hpp"
#include <time.h>

LogMessage::LogMessage(string message, int level) {
	this->level = level;
	this->message = message;
	this->formattedMessage = message + "\n";
	//this->formattedMessage = formatMessage(info);
	//delete info;
}

LogMessage::~LogMessage() {
	level = -1;
	message = "";
	formattedMessage = "";
}

int LogMessage::getLevel() {
	return level;
}

string LogMessage::getMessage() {
	return message;
}

string LogMessage::toString() {
	return formattedMessage;
}

/*string LogMessage::formatMessage(CallerInfo* info) {
	return info->getTime() + " | " + info->getProcessName() + " ("+ info->getPID() + ") | " + message;
}*/
