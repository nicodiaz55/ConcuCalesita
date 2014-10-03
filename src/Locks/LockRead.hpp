/*
 * LockRead.h
 *
 *  Created on: Oct 3, 2014
 *      Author: juan
 */

#ifndef LOCKREAD_H_
#define LOCKREAD_H_

#include "LockFile.h"

class LockRead: public LockFile {
public:
	LockRead(std::string nombre): LockFile ( nombre ){ };

	int tomarLock (){
		this->fl.l_type = F_RDLCK;
		return fcntl ( this->fd,F_SETLKW,&(this->fl) );
	}

	virtual ~LockRead() {};
};

#endif /* LOCKREAD_H_ */
