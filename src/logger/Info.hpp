/*
 * Info.hpp
 *
 *  Created on: 18/9/2014
 *      Author: nicolas
 */

#ifndef INFO_HPP_
#define INFO_HPP_

using namespace std;

#include <string>
#include <sstream>
#include <ctime>

class Info {
	public:
		string name;
		string pid;
	public:
		Info(pid_t pid, string name) {
			this->name = name;
			ostringstream str_pid;
			str_pid << pid;
			this->pid = str_pid.str();
		}
		virtual ~Info() {
			name = "";
			pid = "";
		}
		string calculateTime() {
			time_t rawtime;
			struct tm* timeinfo;
			time(&rawtime);
			timeinfo = localtime(&rawtime);
			char buffer[50];
			strftime(buffer, 50, "%H:%M:%S", timeinfo);
			return string(buffer);
		}
};


#endif /* INFO_HPP_ */
