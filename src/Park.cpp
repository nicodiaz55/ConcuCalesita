/*
 * Park.cpp
 *
 *  Created on: 22/9/2014
 *      Author: nicolas
 */

#include "Park.hpp"
#include "KidsGenerator.hpp"
#include <unistd.h>
#include <sys/wait.h>

Park::Park(unsigned int totalKids, unsigned int kidsMedian) {
	this->logger = Logger::getLogger();
	this->logger->setOutput("test.log");
	this->logger->init();
	this->totalKids = totalKids;
	this->kidsMedian = kidsMedian;
	this->info = new Info(getpid(), "Park");
	this->childrens = 0;
}

Park::~Park() {
	if (logger != NULL) {
		delete logger;
		logger = NULL;
	}
	if (info != NULL) {
		delete info;
		info = NULL;
	}
}

pid_t Park::createKidsGenerator() {
	KidsGenerator generator (kidsMedian, totalKids);
	logger->log("KidsGenerator created", info);
	pid_t pidGen = generator.generate();
	childrens++;
	return pidGen;
}

int Park::open() {
	logger->log("Park is open", info);

	// crear las demas cosas

	pid_t pidGen = createKidsGenerator();
	if (pidGen == CHILD)
		return CHILD;
	return OK;
}



void Park::close() {
	while(childrens > 0) {
		wait(NULL);
		childrens--;
	}
	logger->log("My job here is done.", info);
}
