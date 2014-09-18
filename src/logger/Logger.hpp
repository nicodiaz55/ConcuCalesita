/*
 * Logger.hpp
 *
 *  Created on: 16/9/2014
 *      Author: nicolas
 */

#ifndef LOGGER_HPP_
#define LOGGER_HPP_

using namespace std;

#include "Output.hpp"
#include "Info.hpp"

class Logger {
	private:
		bool canLog;
		Output* output;

	public:
		Logger();
		virtual ~Logger();
		void setOutput(string output);
		void init();
		void stop();
		void log(string message, Info* info);
};



#endif /* LOGGER_HPP_ */
