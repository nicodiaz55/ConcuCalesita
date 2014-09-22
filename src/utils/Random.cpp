/*
 * Random.cpp
 *
 *  Created on: 19/9/2014
 *      Author: nicolas
 */

#include "Random.hpp"
#include <cstdlib>
#include <ctime>
#include <cmath>

void initRandom() {
	srand(time(NULL));
}

unsigned int uniform(unsigned int min, unsigned int max) {
	return (min + rand() % (max - min + 1));
}

float exponentialTime(float median) {
	float u;
	do {
		u = rand() % 1000;
		u /= 1000;
	} while (u == 1);

	return ((-median) * log(1-u));
}
