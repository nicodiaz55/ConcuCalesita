/*
 * KidsGenerator.hpp
 *
 *  Created on: 19/9/2014
 *      Author: nicolas
 */

#ifndef KIDSGENERATOR_HPP_
#define KIDSGENERATOR_HPP_

#include "logger/Logger.hpp"

class KidsGenerator {
	private:
		float median;
		unsigned int max;
		unsigned int kidID;
		Logger* logger;
	public:
		KidsGenerator(float median, unsigned int maxKids);
		virtual ~KidsGenerator();
		pid_t generate();
};



#endif /* KIDSGENERATOR_HPP_ */
