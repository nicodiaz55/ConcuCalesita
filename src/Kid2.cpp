/*
 * Kid.cpp
 *
 *  Created on: Sep 26, 2014
 *      Author: juan
 */

#ifdef KID

#include<iostream>
#include <stdio.h>

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/wait.h>
# include <fcntl.h>

#include <unistd.h>
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

bool numeroAtt(int Id){

	shmid_ds estado;
	shmctl ( Id,IPC_STAT,&estado );
	return estado.shm_nattch;

}


int main ( int argc, char** argv){

	srand(time(NULL));

	//recibe pipes
	int fdRead,fdWrite,fdWrPuerta1,fdWrPuerta2;

	stringstream ss;
	ss.str("");
	ss.clear();
	ss << argv[1];
	ss >> fdRead;

	ss.str("");
	ss.clear();
	ss << argv[2];
	ss >> fdWrPuerta1;

	ss.str("");
	ss.clear();
	ss << argv[3];
	ss >> fdWrPuerta2;

	ss.str("");
	ss.clear();
	ss << argv[4];
	ss >> fdWrite;

	int keyS = ftok("arch",22);
perror("0 ");
	//para hacer operaciones del semaforo
	struct sembuf operations[1];
perror("1 ");
	//para la memoria compartida
	int keyShM = ftok("arch",33);
perror("2 ");
//todo considerar permisos, hacerlos restrictivos
	int semId = semget( keyS, 2, IPC_CREAT|0666); //2 semaforos, cola de ninios y lugares calecita
	perror("3 ");

	int shMId = shmget( keyShM, sizeof(int), IPC_CREAT|0666); //Memoria compartida para aumentar cantidad de chicos
	void* shMp = shmat(shMId,NULL,0);

	int* kidsInPark = static_cast<int*> (shMp);


	//prepara lock de kidsInPark
	struct flock fl;

	fl . l_type = F_WRLCK ;
	fl . l_whence = SEEK_SET ;
	fl . l_start = 0;
	fl . l_len = 0;
	fl . l_pid = getpid () ;
	int fd = open ( "arch" , O_CREAT|O_WRONLY ,0777);

	//y aca arranca
	bool otraVuelta=true;

	while (otraVuelta) {
		//aca consideramos que entro al parque
		fl . l_type = F_WRLCK ;
		fcntl ( fd , F_SETLKW ,&fl );

		(*kidsInPark)++;
cout<< "Cant niños kid:" << (*kidsInPark) << endl;
		fl . l_type = F_UNLCK ;
		fcntl ( fd , F_SETLK ,&fl );

		//se mete en la cola de boletos

		cout<< "Me encole! "<< getpid() << endl;
//Meterse es pasarle a la puerta por donde le tiene que escribir para desbloquearlo
		write(fdWrPuerta1, &fdWrite, sizeof(int));
cout<< "Le escribi a la puerta " << fdWrite << endl;

//Espera que la puerta le escriba "pasa"
		char buf[5];
		int leidos = read(fdRead, buf, 5);

//todo sacar numeros magicos

		cout<< "Termine la cola! "<< getpid() << endl;


		/*
		 *
		 * TODO hace lo que haga despues de sacar boleto y antes de subir a calesita
		 *
		 *
		 */


		//intenta subir a la calecita

		operations[0].sem_num = 1;
		operations[0].sem_op = -1;
		operations[0].sem_flg = SEM_UNDO;

		cout<< "Quiero calesita! "<< getpid() << endl;
		semop(semId, operations, 1);


		cout<< "Subí a la calesita! "<< getpid() << endl;

		//espera que la calesita gire
		operations[0].sem_num = 0;
		operations[0].sem_op = -1;
		operations[0].sem_flg = SEM_UNDO;

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
 	int res = shmdt (shMp);

  	if (res == -1)
  		perror("error");

  	if (numeroAtt(shMId) == 0){
  		res = shmctl(shMId, IPC_RMID, NULL);

  		if (res == -1)
  			perror("error");
	}


  	close (fd) ;

  	close (fdRead); close(fdWrite);

  	return 0;

}

#endif


