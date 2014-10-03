#ifndef LOCKFILE_H_
#define LOCKFILE_H_

#include <unistd.h>
#include <fcntl.h>
#include <string>

class LockFile {

protected:
	struct flock fl;
	int fd;
	std::string nombre;

public:
	LockFile ( const std::string nombre );
	virtual ~LockFile();

	virtual int tomarLock () = 0;
	int liberarLock ();
};

#endif /* LOCKFILE_H_ */
