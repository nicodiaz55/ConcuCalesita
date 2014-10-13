/*
 * Constantes.h
 *
 *  Created on: Oct 8, 2014
 *      Author: juan
 */

#ifndef CONSTANTES_H_
#define CONSTANTES_H_

#include <string>

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




static const int MUERTE_POR_ERROR = -99;
static const int MUERTE_POR_SIGINT = -100;


static const int VALOR_PASAR = 5;

static const int VALOR_PASAR2 = 6;

static const int CERRAR = 1;

static const bool LUGAR_LIBRE = false;
static const bool LUGAR_OCUPADO = true;

static const int INICIO_CLAVES_LUGARES = 500;

static const int PERMISOS_USER_RDWR = 0600;

static const std::string ARCH_LOCK_LOG = "archlocklog";
static const std::string ARCH_LOCK_LUGARES = "archlocklugares";

static const int SEM_ADMIN_REC = 28;

#endif /* CONSTANTES_H_ */
