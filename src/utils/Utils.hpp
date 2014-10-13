/*
 * Utils.hpp
 *
 *  Created on: 22/9/2014
 *      Author: nicolas
 */

#ifndef UTILS_HPP_
#define UTILS_HPP_

#include <sstream>
#include <glob.h>
#include <vector>
#include <errno.h>
#include <string.h>
#include <signal.h>

#include "../logger/Logger.hpp"

using namespace std;

template <typename T>
string toString(T number) {
	ostringstream ss;
	ss << number;
	return ss.str();
}

string toString(char* c_string);

int toInt(string str);

int controlErrores1(int res, Logger* logger, Info* info);

int controlErrores2(int res, Logger* logger, Info* info);

#endif /* UTILS_HPP_ */
