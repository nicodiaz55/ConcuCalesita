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
		static bool instanceFlag;
		static Logger* logger;
		bool canLog;
		Output* output;

		LockWrite* lockLog;
	private:
		Logger();

	public:
		static Logger* getLogger();
		~Logger();
		void setOutput(string output);
		void init();
		void stop();
		void log(string message,const Info* info) const;
};

Logger* obtenerLogger();

#endif /* LOGGER_HPP_ */
