/*
 * VectorMemoCompartida.h
 *
 *  Created on: 14/10/2014
 *      Author: Juan
 */

#ifndef MEMORIA_COMPARTIDA_VECTORMEMOCOMPARTIDA_H_
#define MEMORIA_COMPARTIDA_VECTORMEMOCOMPARTIDA_H_

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <string>
#include <vector>

#include "../Constantes.h"


template <class T> class VectorMemoCompartida {

public:
	int largo;
	std::vector<T*> vectPtr;
	std::vector<int> vectshmID;

	long unsigned int cantidadProcesosAdosados () const;

public:
	VectorMemoCompartida ();

	~VectorMemoCompartida ();
	int crear ( const std::string& archivo,const int letra, uint permisos, int largo);
	int liberar ();
	void escribir ( const T& dato, int indice );
	T leer (int indice) const;

};

template <class T> VectorMemoCompartida<T> :: VectorMemoCompartida() : largo(0) {
}

template <class T> VectorMemoCompartida<T> :: ~VectorMemoCompartida() {
}

template <class T> int VectorMemoCompartida<T> :: crear ( const std::string& archivo,const int letra , uint permisos, int largo) {

	if (permisos < 0 || permisos > 0777){
		return RES_ERROR_PERMISOS;
	}

	this->largo = largo;

	int claveAux = INICIO_CLAVES_LUGARES;

	for (int i = 0; i < largo; ++i) {
		// generacion de la clave
		key_t clave = ftok ( archivo.c_str(),claveAux );
		claveAux++;
		if ( clave == -1 )
			return RES_ERROR_FTOK;
		else {
			// creacion de la memoria compartida
			int shmId = shmget ( clave,sizeof(T), permisos|IPC_CREAT );
			vectshmID.push_back(shmId);

			if ( shmId == -1 )
				return RES_ERROR_SHMGET;
			else {
				// attach del bloque de memoria al espacio de direcciones del proceso
				void* ptrTemporal = shmat ( shmId,NULL,0 );

				if ( ptrTemporal == (void *) -1 ) {
					return RES_ERROR_SHMAT;
				} else {
					T* ptrDatos = static_cast<T*> (ptrTemporal);
					vectPtr.push_back(ptrDatos);
				}
			}
		}
	}
	return RES_OK;
}

template <class T> int VectorMemoCompartida<T> :: liberar () {
	// detach del bloque de memoria

	for (int i = 0; i < vectPtr.size(); ++i) {
		int res = shmdt ( static_cast<void*> (vectPtr[i]) );
		if (res != 0){ return RES_ERROR_SHMDT;}
	}

	long unsigned int procAdosados = this->cantidadProcesosAdosados ();

	if ( procAdosados == 0 ) {
		for (int i = 0; i < vectshmID.size(); ++i) {
			int res = shmctl ( vectshmID[i], IPC_RMID, NULL );
			if (res != 0){ return RES_ERROR_SHMCTL;}
		}
	}

	return RES_OK;
}

template <class T> void VectorMemoCompartida<T> :: escribir ( const T& dato, int indice ) {
	* (vectPtr[indice]) = dato;
}

template <class T> T VectorMemoCompartida<T> :: leer ( int indice) const {
	return ( *(vectPtr[indice]) );
}

template <class T> long unsigned int VectorMemoCompartida<T> :: cantidadProcesosAdosados () const {

	shmid_ds estado;
	long unsigned int cantAtt = 0;

	for (int i = 0; i < vectshmID.size(); ++i) {
		shmctl ( vectshmID[i], IPC_STAT, &estado );
		cantAtt = estado.shm_nattch;
		if (cantAtt > 0) {return cantAtt;}
	}

	return cantAtt;

}



#endif /* MEMORIA_COMPARTIDA_VECTORMEMOCOMPARTIDA_H_ */
