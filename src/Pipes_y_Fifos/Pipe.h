#ifndef PIPE_H_
#define PIPE_H_

#include <unistd.h>
#include <fcntl.h>

class Pipe {

private:
	int descriptores[2];
	bool lectura;
	bool escritura;

public:
	static const int LECTURA = 0;
	static const int ESCRITURA = 1;

	Pipe();
	Pipe(int fd1, int fd2): lectura(true),escritura(true) {
		descriptores[0]=fd1;
		descriptores[1]=fd2;
	}

	~Pipe();

	void setearModo ( const int modo );

	ssize_t escribir ( const void* dato,const int datoSize );
	ssize_t leer ( void* buffer,const int buffSize );

	int getFdLectura () const;
	int getFdEscritura () const;

	void cerrar ();
};

#endif /* PIPE_H_ */