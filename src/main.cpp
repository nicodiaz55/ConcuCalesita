/*
 * main.cpp
 *
 *  Created on: 22/9/2014
 *      Author: nicolas
 */

using namespace std;

#include "Park.hpp"
#include <cstdlib>

void simulate(unsigned int totalKids, unsigned int kidsMedian) {
	Park* park = new Park(totalKids, kidsMedian);

	int open = park->open();
	if (open == Park::CHILD)
		return;

	// TODO: esperar a que todos los procesos que cree PARK le informen que terminaron para poder terminar
	park->close();
	delete park;
}

int main(int argc, char* argv[]){

	// Aca iría el parseo de parametros

	unsigned int totalKids = 3;
	unsigned int kidsMedian = 5;

	simulate(totalKids, kidsMedian); // seguro se necesitarán mas parametros

	exit(0);
}
