/*
 * KidsGenerator.cpp
 *
 *  Created on: 19/9/2014
 *      Author: nicolas
 */

#include <unistd.h>
#include "utils/Utils.hpp"
#include "KidsGenerator.hpp"
#include "utils/Random.hpp"
#include "Kid.hpp"

KidsGenerator::KidsGenerator(float median, unsigned int maxKids) {
	this->median = median;
	this->max = maxKids;
	this->kidID = 1;
	this->logger = Logger::getLogger();
}

KidsGenerator::~KidsGenerator() {
	median = 0;
	max = 0;
	kidID = 0;
	logger = NULL; // El delete lo hace el proceso principal
}

pid_t KidsGenerator::generate() {
	pid_t id = fork();
	if (id != 0) {
		return id;
	}

	Info* info = new Info(id, "KidsGenerator");

	initRandom();

	// Child: KidsGenerator
	while (kidID <= max) {
		Kid* kid = new Kid(kidID);
		//TODO: Enviar kid a algun lado.
		logger->log("Created a new kid with ID=" + toString(kidID), info);
		usleep((int) exponentialTime(median));
		kidID++;
		delete kid; //TODO: eliminar esta linea
	}

	logger->log("My job here is done.", info);

	return id;
}
