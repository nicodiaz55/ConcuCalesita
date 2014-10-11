/*
 * LogMessage.cpp
 *
 *  Created on: 16/9/2014
 *      Author: nicolas
 */

#include "LogMessage.hpp"

LogMessage::LogMessage(string message, const Info* info) {
	this->message = message;
	this->formattedMessage = formatMessage(info);
}

LogMessage::~LogMessage() {
	message = "";
	formattedMessage = "";
}

/**
 * Devuelve el mensaje que se loggea
 */
string LogMessage::getMessage() {
	return message;
}

/**
 * Salida con formato, lo que se loggea
 */
string LogMessage::toString() {
	return formattedMessage;
}

/**
 * Da formato al mensaje que se va a loggear
 */
string LogMessage::formatMessage(const Info* info) {
	if (info == NULL)
		return message + "\n";
	return info->calculateTime() + " | [" + info->pid + "] " + info->name + "\t | " + message + "\n";
}
