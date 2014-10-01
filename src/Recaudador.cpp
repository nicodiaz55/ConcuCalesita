/*
 * Recaudador.cpp
 *
 *  Created on: Sep 30, 2014
 *      Author: juan
 */

#ifdef RECAUDADOR

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
  * Registra en la caja el cobro de boletos
  */

using namespace std;


//todo Control de errores!!

bool numeroAtt(int Id){

	shmid_ds estado;
	shmctl ( Id,IPC_STAT,&estado );
	return estado.shm_nattch;

}

//todo cambiar a pasado por parametro
static const double precio = 2.5;

int main ( int argc, char** argv){

	//recibe pipes
	int fdRead;

	stringstream ss;
	ss.str("");
	ss.clear();
	ss << argv[1];
	ss >> fdRead;

	cout << "Soy recaudador y leo del: "<< fdRead << endl;

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

	while (true){

		int AvisoPago;
		read(fdRead, &AvisoPago, sizeof(int));

	cout << "Recaud. leyo: "<< AvisoPago << endl;
		if (AvisoPago == 2) {break;}


		fl . l_type = F_WRLCK ;
		fcntl ( fd , F_SETLKW ,&fl );

		(*caja)+=precio;
		cout<<"En caja hay: " << (*caja) << endl;

		fl . l_type = F_UNLCK ;
		fcntl ( fd , F_SETLK ,&fl );

	}

	//para que muera el administrador
	fl . l_type = F_WRLCK ;
	fcntl ( fd , F_SETLKW ,&fl );
	(*caja)=-1;
	fl . l_type = F_UNLCK ;
	fcntl ( fd , F_SETLK ,&fl );

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


