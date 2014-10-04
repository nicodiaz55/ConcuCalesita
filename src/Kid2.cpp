/*
 * Kid.cpp
 *
 *  Created on: Sep 26, 2014
 *      Author: juan
 */

#ifdef KID

#include<iostream>
#include <stdio.h>


#include <sys/sem.h>

#include "Memoria_Compartida/MemoriaCompartida.h"
#include "Pipes_y_Fifos/Pipe.h"
#include "Pipes_y_Fifos/FifoLectura.h"
#include "Locks/LockWrite.hpp"
#include "Locks/LockRead.hpp"

#include <stdlib.h>
#include <errno.h>
#include "string.h"

#include "logger/Logger.hpp"
#include "utils/Utils.hpp"


 /*
  * Kid
  *
  * Aumenta KidsInPark al comenzar con shMem y locks
  *	Se mete en la cola para comprar boleto con pipe
  *	Corre para subir a la calesita con semaforo
  *
  */

using namespace std;

//todo Control de errores!!



int main ( int argc, char** argv){

	srand(time(NULL));

	//recibe pipes
	int fdRdPuerta1,fdWrPuerta1;

	stringstream ss;
	ss.str("");
	ss.clear();
	ss << argv[1];
	ss >> fdRdPuerta1;

	ss.str("");
	ss.clear();
	ss << argv[2];
	ss >> fdWrPuerta1;

	Pipe pipePuerta1(fdRdPuerta1,fdWrPuerta1);

	pipePuerta1.setearModo(Pipe::ESCRITURA);

	string ruta = "Cola" + toString(getpid());
	FifoLectura cola(ruta);


	//para la memoria compartida
	MemoriaCompartida<int> kidsInPark;
	kidsInPark.crear("arch",33); //todo permisos!


//todo considerar permisos, hacerlos restrictivos
//todo encapsular semaforos (esperar a que den el codigo en la practica)
	//para hacer operaciones del semaforo
	struct sembuf operations[1];

	int key = ftok("arch",22);
	int key2 = ftok("arch",23);
	int key3 = ftok("arch",24);

	int semId = semget( key, 1, IPC_CREAT|0666);
	int semId2 = semget( key2, 1, IPC_CREAT|0666);
	int semId3 = semget( key3, 1, IPC_CREAT|0666); //mutex entrada y salida

	//prepara lock de kidsInPark
	LockFile* lockW = new LockWrite("archLockKids");

	//y aca arranca
	bool otraVuelta=true;

	while (otraVuelta) {
		//aca consideramos que entro al parque

		//para que entre "de a uno" uso semaforo binario para seccion critica
		operations[0].sem_num = 0;
		operations[0].sem_op = -1;
		operations[0].sem_flg = 0;
		semop(semId3, operations, 1);

		//todo loggear que entro

		//como entro aumenta cantidad de chicos presentes...
		lockW->tomarLock();

		kidsInPark.escribir(kidsInPark.leer()+1);
cout<< "Cant niños kid:" << kidsInPark.leer() << endl;

		lockW->liberarLock();


		operations[0].sem_num = 0;
		operations[0].sem_op = 1;
		operations[0].sem_flg = 0;
		semop(semId3, operations, 1);

		//se mete en la cola de boletos

		cout<< "Me encole! "<< getpid() << endl;
//Meterse es pasarle a la puerta por donde le tiene que escribir para desbloquearlo
		int pid = getpid();
		pipePuerta1.escribir( &pid, sizeof(int) );

cout<< "Le escribi a la puerta " << ruta << endl;

//Espera que la puerta le escriba "pasa"
		int permisoPasar = 0;
		cola.leer(&permisoPasar, sizeof(int) );

//todo sacar numeros magicos

		cout<< "Termine la cola! boleto"<< getpid() << " Lei: "<< permisoPasar <<endl;

//Espera que la segunda puerta le escriba "pasa"
		cola.leer(&permisoPasar, sizeof(int) );

		cout<< "Poria subir a calesita... "<< getpid() << " Lei: "<< permisoPasar <<endl;

		//intenta subir a la calecita

		operations[0].sem_num = 0;
		operations[0].sem_op = -1;
		operations[0].sem_flg = 0;

		cout<< "Quiero calesita! "<< getpid() << endl;

		/************DEBUG***********************/
		ushort arreglo2[2];

		union semnum {
			int val;
			struct semid_ds* buf;
			ushort* array;
		};

		semnum init2;
		init2.array=arreglo2;
		semctl(semId2,0,GETPID,init2);
		cout << semctl(semId2,0,GETALL,init2)<<endl;
		cout<< "Semaf calesita lugares libres: " << init2.array[0] << "Tocado por: " << init2.val << endl;

		/************************************************************/

		semop(semId2, operations, 1);


		cout<< "Subí a la calesita! "<< getpid() << endl;

		//espera que la calesita gire
		operations[0].sem_num = 0;
		operations[0].sem_op = -1;
		operations[0].sem_flg = 0;

		/************DEBUG***********************/
		ushort arreglo[2];


		semnum init;
		init.array=arreglo;
		semctl(semId,0,GETALL,init);
		cout<< "Avisos calesita girando: " << init.array[0] << endl;


		/************************************************************/

		semop(semId, operations, 1);

		cout<< "Me bajé! "<< getpid() << endl;

		otraVuelta = false;
		//A veces el chico quiere subirse de nuevo
	/*
		if ((rand()%100) > 20){
			otraVuelta = true;
		}
	*/
	}

	//libero memoria compartida
	kidsInPark.liberar();

  	//cierro pipe a las puerta y fifo de la cola
	pipePuerta1.cerrar();

	cola.cerrar();
	cola.eliminar();

	delete lockW;

	//para que salga "de a uno" uso semaforo binario para seccion critica, uso el mismo de la entrada
	// -> no salen y entran al mismo tiempo
	operations[0].sem_num = 0;
	operations[0].sem_op = -1;
	operations[0].sem_flg = 0;
	semop(semId3, operations, 1);

	//todo loggear que salio
	//todo preguntar si habri que hacer algo particular aca...

	operations[0].sem_num = 0;
	operations[0].sem_op = 1;
	operations[0].sem_flg = 0;
	semop(semId3, operations, 1);
  	return 0;

}

#endif


