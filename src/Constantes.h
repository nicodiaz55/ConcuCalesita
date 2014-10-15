/*
 * Constantes.h
 *
 *  Created on: Oct 8, 2014
 *      Author: juan
 */

#ifndef CONSTANTES_H_
#define CONSTANTES_H_

#include <string>

//Errores
static const int RES_OK = 0;

static const int RES_PARAM_NUM_ERR = -1;

static const int RES_PARAM_INV = -2;

static const int RES_ERROR_PERMISOS = -3;

static const int RES_ERROR_SIGADDSET = -4;

static const int RES_ERROR_SIGACTION = -5;

static const int RES_ERROR_FTOK	= -6;

static const int RES_ERROR_SHMGET = -7;

static const int RES_ERROR_SHMAT = -8;

static const int RES_ERROR_SEMGET = -9;

static const int RES_ERROR_SHMCTL = -10;

static const int RES_ERROR_SHMDT = -11;

static const int RES_ERROR_PIPE = -12;

static const int RES_ERROR_SEMCTL = -13;

static const int RES_ERROR_SEMOP = -14;

static const int RES_ERROR_VALOR_P_V = -15;

static const int RES_ERROR_VALOR_CAJA = -16;

static const int RES_ERROR_CANT_PROC_ADOSADOS = -17;



static const int MUERTE_POR_ERROR = -99;
static const int MUERTE_POR_SIGINT = -100;


//valores arbitrarios que se pasan por fifos y pipes
static const int VALOR_PASAR = 5;
static const int VALOR_PASAR2 = 6;
static const int AVISO_DE_PAGO = 1;

static const int CERRAR = 1;
static const int CERRAR_FILA = -1;
static const int FIN_PAGOS = 2;


//valores de estado de los lugares de la calesita
static const bool LUGAR_LIBRE = false;
static const bool LUGAR_OCUPADO = true;

//permisos
static const int PERMISOS_USER_RDWR = 0600;

//archivos de locks
static const std::string ARCH_SEM = "/etc";
static const std::string ARCH_LOCK_CAJA = "archLockCaja";
static const std::string ARCH_LOCK_LOG = "archlocklog";
static const std::string ARCH_LOCK_LUGARES = "archlocklugares";
static const std::string ARCH_LOCK_CONTINUA = "archlockCont";
static const std::string ARCH_LOCK_KIDS = "archlockKids";

//Fifos
static const std::string PREFIJO_FIFO_FILA_KIDS = "Cola";
static const std::string SUFIJO_FIFO_FILA_KIDS = "C";
static const std::string FIFORECAUDADOR = "FifoRecaudador";

//claves de semaforos y memorias
static const int INICIO_CLAVES_LUGARES = 500;
static const int SEM_CAL_GIRA = 22;
static const int SEM_MUTEX_ENTR = 24;
static const int SEM_CAL_LUG = 23;
static const int SEM_COLA_CAL = 25;
static const int SEM_CAL_SUBIR = 29;
static const int SEM_ADMIN_REC = 28;

static const int MEM_KIDS = 33;
static const int SEM_CAJA_REC = 44;
static const int MEM_CONTINUA = 55;
static const int SEM_CAJA_ESTADO = 66;
#endif /* CONSTANTES_H_ */
