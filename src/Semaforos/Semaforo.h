#ifndef SEMAFORO_H_
#define SEMAFORO_H_

#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <string>

#include "../Constantes.h"

class Semaforo {

private:
	std::string arch;
	int clave;
	int valorInicial; // valor inical al ser creado o -1 si se crea ya inicializado

	int nombre;
	int inicializar () const;

public:
	Semaforo ( const std::string& nombre,int clave,const int valorInicial );
	Semaforo ( const std::string& nombre,int clave);
	~Semaforo();

	int crear();
	int p (int cant) const; // decrementa
	int v (int cant) const; // incrementa
	int zero () const; //espera a cero

	int eliminar () const;
};

#endif /* SEMAFORO_H_ */
