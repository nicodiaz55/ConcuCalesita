/*
 * LogMessage.hpp
 *
 *  Created on: 16/9/2014
 *      Author: nicolas
 */

#ifndef LOGMESSAGE_HPP_
#define LOGMESSAGE_HPP_

#include <string>

using namespace std;

class LogMessage {
	private:
		int level;
		string message;
		string formattedMessage;
	public:
		LogMessage(string message, int level);
		virtual ~LogMessage();
		int getLevel();
		string getMessage();
		string toString();
	private:
		//string formatMessage(CallerInfo* info);
};



#endif /* LOGMESSAGE_HPP_ */
