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
//#include "../Locks/LockWrite.hpp"
#include <sys/sem.h>

class Output {
	private:
		string output;
		int file; // file descriptor

		int semId;//cosas del semaforo para que sea concurrente el output
		struct sembuf operations[1];
	public:
		Output(const string output);
		virtual ~Output();
		bool init();
		void log(LogMessage* message);
};


#endif /* OUTPUT_HPP_ */
