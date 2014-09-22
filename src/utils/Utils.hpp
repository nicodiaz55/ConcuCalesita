/*
 * Utils.hpp
 *
 *  Created on: 22/9/2014
 *      Author: nicolas
 */

#ifndef UTILS_HPP_
#define UTILS_HPP_

#include <sstream>

using namespace std;

template <typename T>
string toString(T number) {
	ostringstream ss;
	ss << number;
	return ss.str();
}



#endif /* UTILS_HPP_ */
