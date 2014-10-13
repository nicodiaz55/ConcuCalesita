/*
 * Utils.cpp
 *
 *  Created on: Oct 12, 2014
 *      Author: juan
 */

#include "Utils.hpp"

string toString(char* c_string) {
	stringstream ss;
	ss << c_string;
	return ss.str();
}

int toInt(string str) {
	stringstream ss;
	ss.str("");
	ss.clear();
	ss << str;

	int returnVal = 0;
	ss >> returnVal;
	return returnVal;
}

int controlErrores1(int res, Logger* logger, Info* info) {
	if (res != RES_OK) {
		logger->log(
				"Error: " + toString(res) + ". Strerr: "
				+ toString(strerror(errno)), info);
		return MUERTE_POR_ERROR;
	}
	return RES_OK;
}

int controlErrores2(int res, Logger* logger, Info* info) {
	if (res == -1) {
		logger->log(
				"Error: " + toString(res) + ". Strerr: "
				+ toString(strerror(errno)), info);
		return MUERTE_POR_ERROR;
	}
	return RES_OK;
}


