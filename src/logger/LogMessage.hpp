/*
 * LogMessage.hpp
 *
 *  Created on: 16/9/2014
 *      Author: nicolas
 */

#ifndef LOGMESSAGE_HPP_
#define LOGMESSAGE_HPP_

#include <string>
#include "Info.hpp"

using namespace std;

class LogMessage {
	private:
		string message;
		string formattedMessage;
	public:
		LogMessage(string message, Info* info);
		virtual ~LogMessage();
		string getMessage();
		string toString();
	private:
		string formatMessage(Info* info);
};



#endif /* LOGMESSAGE_HPP_ */
