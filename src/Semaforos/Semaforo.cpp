#include "Semaforo.h"

Semaforo :: Semaforo ( const std::string& nombre, int numero,const int valorInicial ):
arch(nombre), clave(numero), valorInicial(valorInicial), nombre(0){

}

Semaforo :: Semaforo ( const std::string& nombre,int numero):
arch(nombre), clave(numero), valorInicial(-1), nombre(0){

}

Semaforo::~Semaforo() {
}

int Semaforo:: crear(){

	key_t key = ftok ( arch.c_str(),clave );

	if (clave == -1) { return RES_ERROR_FTOK;}

	this->nombre = semget ( key,1,0666 | IPC_CREAT );

	if (nombre < 0) { return RES_ERROR_SEMGET;}

	this->inicializar ();

	return 0;

}

int Semaforo :: inicializar () const {

	union semnum {
		int val;
		struct semid_ds* buf;
		ushort* array;
	};

	semnum init;
	init.val = this->valorInicial;
	int resultado = semctl ( this->nombre,0,SETVAL,init );
	return resultado;
}

int Semaforo :: p (int cant) const {

	struct sembuf operacion;

	operacion.sem_num = 0;	// numero de semaforo
	operacion.sem_op  = cant;	// intentar restar cant al semaforo
	operacion.sem_flg = 0;

	int resultado = semop ( this->nombre,&operacion,1 );

	if(resultado != RES_OK){
		return RES_ERROR_SEMOP;
	}

	return RES_OK;
}

int Semaforo :: zero () const {

	struct sembuf operacion;

	operacion.sem_num = 0;	// numero de semaforo
	operacion.sem_op  = 0;	// espera a que llegue a cero
	operacion.sem_flg = 0;

	int resultado = semop ( this->nombre,&operacion,1 );

	if(resultado != RES_OK){
		return RES_ERROR_SEMOP;
	}

	return RES_OK;
}

int Semaforo :: v (int cant) const {
//todo cntrolar cant sea positiva o negativa en p y v
	struct sembuf operacion;

	operacion.sem_num = 0;	// numero de semaforo
	operacion.sem_op  = cant;	// sumar cant al semaforo
	operacion.sem_flg = 0;

	int resultado = semop ( this->nombre,&operacion,1 );

	if(resultado != RES_OK){
		return RES_ERROR_SEMOP;
	}

	return RES_OK;
}

int Semaforo :: eliminar () const {
	int res = semctl ( this->nombre,0,IPC_RMID );

	if (res != RES_OK){ return RES_ERROR_SEMCTL;}

	return RES_OK;
}
