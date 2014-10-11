/*
 * Logger.hpp
 *
 *  Created on: 16/9/2014
 *      Author: nicolas
 */

#ifndef LOGGER_HPP_
#define LOGGER_HPP_

using namespace std;

#include <iostream>
#include "Output.hpp"
#include "Info.hpp"
#include "../Constantes.h"

#include "../Locks/LockWrite.hpp"

class Logger {
	private:
		bool canLog;
		Output* output;
		LockWrite* lockLog;
	public:
		Logger();
		~Logger();
		void init();
		void stop();
		void setOutput(string output);
		void log(string message,const Info* info) const;
		void start(); // mensaje de entrada
		void end(); // mensaje de salida
};

#endif /* LOGGER_HPP_ */
