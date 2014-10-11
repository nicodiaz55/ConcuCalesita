/*
 * Utils.hpp
 *
 *  Created on: 22/9/2014
 *      Author: nicolas
 */

#ifndef UTILS_HPP_
#define UTILS_HPP_

#include <sstream>
#include <glob.h>
#include <vector>
#include <errno.h>
#include <string.h>
using namespace std;

template <typename T>
string toString(T number) {
	ostringstream ss;
	ss << number;
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
		return MUERTE_POR_ERROR;
	}
	return RES_OK;
}

#endif /* UTILS_HPP_ */
