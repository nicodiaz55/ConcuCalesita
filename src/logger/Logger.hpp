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

class Logger {
	private:
		int level;
		Output* output;

	public:
		Logger();
		virtual ~Logger();
		void setOutput(Output* output);
		void setLevel(int level);
		void log(string message, int level);

	private:
		bool isPublishable(LogMessage* message);
		LogMessage* createLogMessage(string message, int level);
};



#endif /* LOGGER_HPP_ */
