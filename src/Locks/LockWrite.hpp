/*
 * LockWrite.h
 *
 *  Created on: Oct 3, 2014
 *      Author: juan
 */

#ifndef LOCKWRITE_H_
#define LOCKWRITE_H_

#include "LockFile.h"

class LockWrite: public LockFile {
public:
	LockWrite(std::string nombre): LockFile ( nombre ){ };

	int tomarLock (){
		this->fl.l_type = F_WRLCK;
		return fcntl ( this->fd,F_SETLKW,&(this->fl) );
	}

	ssize_t escribir ( const void* buffer,const ssize_t buffsize ) const {
		lseek ( this->fd,0,SEEK_END );
		return write ( this->fd,buffer,buffsize );
	}

	virtual ~LockWrite() {};
};

#endif /* LOCKWRITE_H_ */





