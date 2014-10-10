/*
 * Output.hpp
 *
 *  Created on: 16/9/2014
 *      Author: nicolas
 */

#ifndef OUTPUT_HPP_
#define OUTPUT_HPP_

using namespace std;

#include "LogMessage.hpp"


class Output {
	private:
		string output;
		int file; // file descriptor

	public:
		Output(const string output);
		virtual ~Output();
		bool init();
		void log(LogMessage* message);
};


#endif /* OUTPUT_HPP_ */
