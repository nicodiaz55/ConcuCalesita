/*
 * Park.hpp
 *
 *  Created on: 22/9/2014
 *      Author: nicolas
 */

#ifndef PARK_HPP_
#define PARK_HPP_

using namespace std;

#include "logger/Logger.hpp"
#include "logger/Info.hpp"

class Park {
	private:
		unsigned int totalKids;
		unsigned int kidsMedian;
		unsigned int childrens;
		Info* info;
		Logger* logger;
	public:
		static const int CHILD = 0;
		static const int OK = 1;
	private:
		pid_t createKidsGenerator();
		// creadores de otros procesos (Admin, Boleteria, Calesita...)
	public:
		Park(unsigned int totalKids, unsigned int kidsMedian);
		~Park();
		int open();
		void close();

};



#endif /* PARK_HPP_ */
