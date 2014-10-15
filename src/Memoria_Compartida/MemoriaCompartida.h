#ifndef MEMORIACOMPARTIDA_H_
#define MEMORIACOMPARTIDA_H_

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <string>

#include "../Constantes.h"


template <class T> class MemoriaCompartida {

public:
	int shmId;
	T*	ptrDatos;

	long unsigned int cantidadProcesosAdosados () const;

public:
	MemoriaCompartida ();
	MemoriaCompartida (const MemoriaCompartida  &shMem);

	~MemoriaCompartida ();
	int crear ( const std::string& archivo,const int letra, uint permisos);
	int liberar ();
	void escribir ( const T& dato );
	T leer () const;

};

template <class T> MemoriaCompartida<T> :: MemoriaCompartida() : shmId(0), ptrDatos(NULL) {
}

template <class T> MemoriaCompartida<T> :: MemoriaCompartida (const MemoriaCompartida  &shMem) {
   this->shmId = shMem.shmId;
   this->ptrDatos = shMem.ptrDatos;
}

template <class T> MemoriaCompartida<T> :: ~MemoriaCompartida() {
}

template <class T> int MemoriaCompartida<T> :: crear ( const std::string& archivo,const int letra , uint permisos) {

	if (permisos < 0 || permisos > 0777){
		return RES_ERROR_PERMISOS;
	}

	// generacion de la clave
	key_t clave = ftok ( archivo.c_str(),letra );
	if ( clave == -1 )
		return RES_ERROR_FTOK;
	else {
		// creacion de la memoria compartida
		this->shmId = shmget ( clave,sizeof(T), permisos|IPC_CREAT );

		if ( this->shmId == -1 )
			return RES_ERROR_SHMGET;
		else {
			// attach del bloque de memoria al espacio de direcciones del proceso
			void* ptrTemporal = shmat ( this->shmId,NULL,0 );

			if ( ptrTemporal == (void *) -1 ) {
				return RES_ERROR_SHMAT;
			} else {
				this->ptrDatos = static_cast<T*> (ptrTemporal);
				return RES_OK;
			}
		}
	}
}

template <class T> int MemoriaCompartida<T> :: liberar () {
	// detach del bloque de memoria
	int res = shmdt ( static_cast<void*> (this->ptrDatos) );

	if (res != 0){ return RES_ERROR_SHMDT;}

	long unsigned int procAdosados = this->cantidadProcesosAdosados ();

	if (procAdosados == -1){return RES_ERROR_CANT_PROC_ADOSADOS;}

	if ( procAdosados == 0 ) {
		res = shmctl ( this->shmId,IPC_RMID,NULL );
		if (res != 0){ return RES_ERROR_SHMCTL;}
	}
	return RES_OK;
}

template <class T> void MemoriaCompartida<T> :: escribir ( const T& dato ) {
	* (this->ptrDatos) = dato;
}

template <class T> T MemoriaCompartida<T> :: leer () const {
	return ( *(this->ptrDatos) );
}

template <class T> long unsigned int MemoriaCompartida<T> :: cantidadProcesosAdosados () const {
	shmid_ds estado;
	int res = shmctl ( this->shmId,IPC_STAT,&estado );
	if (res == -1) { return res;}
	return estado.shm_nattch;
}


#endif /* MEMORIACOMPARTIDA_H_ */
