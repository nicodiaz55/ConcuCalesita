/*
 * Administrador.cpp
 *
 *  Created on: Oct 1, 2014
 *      Author: juan
 */



#ifdef ADMIN

#include<iostream>
#include <stdio.h>

#include <sys/types.h>
#include <sys/ipc.h>
#include <signal.h>
#include <memory.h>
# include <fcntl.h>
#include <sys/shm.h>

#include <unistd.h>
#include <stdlib.h>

#include "logger/Logger.hpp"
#include "utils/Utils.hpp"
 /*
  * Mira la caja de boletos e imprime por pantalla o al log o whatever
  */

using namespace std;

//todo Control de errores!!


bool numeroAtt(int Id){

	shmid_ds estado;
	shmctl ( Id,IPC_STAT,&estado );
	return estado.shm_nattch;

}


int main ( int argc, char** argv){

	//recibe pipe
	int fdRead;

	//pide memoria comp. para caja

	int keyShM = ftok("arch",44);
	int shMId = shmget( keyShM, sizeof(int), IPC_CREAT|0666); //por ahora memoria compartida para la caja
	void* shMp = shmat(shMId,NULL,0);

	int* caja = static_cast<int*> (shMp);

	//prepara lock de caja recaudacion
	struct flock fl;

	fl . l_type = F_WRLCK ;
	fl . l_whence = SEEK_SET ;
	fl . l_start = 0;
	fl . l_len = 0;
	fl . l_pid = getpid () ;
	int fd = open ( "arch2" , O_CREAT|O_WRONLY ,0777);
cout<< "fd lock caja: "<< fd <<endl;
	while (true){

		fl . l_type = F_WRLCK ;
		fcntl ( fd , F_SETLKW ,&fl );

		if ((*caja) >= 0){
			cout<<"Admin: \"En caja hay:\" " << (*caja) << endl;
		}else{
			break;
		}

		fl . l_type = F_UNLCK ;
		fcntl ( fd , F_SETLK ,&fl );

		sleep(1);

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

	close(fd);


return 0;

}

#endif

