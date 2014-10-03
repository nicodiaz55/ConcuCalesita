#include "LockFile.h"

LockFile :: LockFile ( const std::string nombre ) {

	this->nombre = nombre;
	this->fl.l_type = F_WRLCK;
	this->fl.l_whence = SEEK_SET;
	this->fl.l_start = 0;
	this->fl.l_len = 0;
	this->fd = open ( this->nombre.c_str(),O_CREAT|O_RDWR,0777 );
}

int LockFile :: liberarLock () {
	this->fl.l_type = F_UNLCK;
	return fcntl ( this->fd,F_SETLK,&(this->fl) );
}


LockFile :: ~LockFile () {
	//libera el lock si esta tomado y borra el archivo usado por el lock (si no hay referencias a el)
	this->fl.l_type = F_UNLCK;
	fcntl ( this->fd,F_SETLK,&(this->fl) );

	close ( this->fd );
	unlink ( nombre.c_str() );
}
