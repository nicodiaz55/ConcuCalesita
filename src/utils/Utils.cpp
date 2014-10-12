/*
 * Utils.cpp
 *
 *  Created on: Oct 12, 2014
 *      Author: juan
 */

#include "Utils.hpp"

string toString(char* c_string) {
	stringstream ss;
	ss << c_string;
	return ss.str();
}

int toInt(string str) {
	stringstream ss;
	ss.str("");
	ss.clear();
	ss << str;

	int returnVal = 0;
	ss >> returnVal;
	return returnVal;
}

vector<string> glob(const string& pat){
    glob_t glob_result;
    glob(pat.c_str(),GLOB_TILDE,NULL,&glob_result);
    vector<string> ret;
    for(unsigned int i=0;i<glob_result.gl_pathc;++i){
        ret.push_back(string(glob_result.gl_pathv[i]));
    }
    globfree(&glob_result);
    return ret;
}

int controlErrores1(int res, Logger* logger, Info* info) {
	if (res != RES_OK) {
		logger->log(
				"Error: " + toString(res) + ". Strerr: "
				+ toString(strerror(errno)), info);
		kill(getppid(), SIGINT);
	}
	return RES_OK;
}

int controlErrores2(int res, Logger* logger, Info* info) {
	if (res == -1) {
		logger->log(
				"Error: " + toString(res) + ". Strerr: "
				+ toString(strerror(errno)), info);
		kill(getppid(), SIGINT);
	}
	return RES_OK;
}


