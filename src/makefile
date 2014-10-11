
CC = g++
CFLAGS = -Wall -Wpedantic

INC = $(.) $(./Locks) $(./logger) $(./Memoria_Compartida) $(./Pipes_y_Fifos) $(./Seniales) $(./utils) $(./Semaforos)
INC_PARAMS = $(foreach d, $(INC), -I$d)

MEM = ./Memoria_Compartida/MemoriaCompartida.h
PIPE = ./Pipes_y_Fifos/Pipe.h
CONST = Constantes.h
LOG = ./logger/Logger.hpp
UTILS = ./utils/Utils.hpp
CAJA = ./Caja.h
FIFORD = ./Pipes_y_Fifos/FifoLectura.h
FIFOWR = ./Pipes_y_Fifos/FifoEscritura.h
SIG = ./Seniales/SignalHandler.h
LOCKRD = ./Locks/LockRead.hpp
LOCKWR = ./Locks/LockWrite.hpp
SEM = ./Semaforos/Semaforo.h
RANDOM = ./utils/Random.hpp

DEPS_LANZADOR = $(MEM) $(PIPE) $(CONST) $(LOG) $(UTILS) $(SEM) $(SIG) $(RANDOM) $(CAJA)
DEPS_ADMINISTRADOR = $(CONST) $(LOG) $(UTILS) $(MEM) $(LOCKRD) $(RANDOM) $(CAJA)
DEPS_CALESITA = $(CONST) $(LOG) $(UTILS) $(MEM) $(LOCKWR) $(LOCKRD) $(SEM)
DEPS_KID = $(CONST) $(LOG) $(UTILS) $(MEM) $(PIPE) $(FIFORD) $(LOCKWR) $(LOCKRD) $(SEM)
DEPS_RECAUDADOR = $(MEM) $(FIFORD) $(LOCKWR) $(LOCKRD) $(CONST) $(LOG) $(UTILS) $(CAJA)
DEPS_FILACALESITA = $(CONST) $(LOG) $(UTILS) $(SIG) $(FIFOWR) $(PIPE)
DEPS_FILABOLETO = $(CONST) $(LOG) $(UTILS) $(SIG) $(FIFOWR) $(PIPE) $(SEM)
DEPS_FANTASMA = $(CONST) $(LOG) $(UTILS) $(LOCKWR) $(MEM) $(SEM)

OBJ = Pipe.o Fifo.o FifoLectura.o FifoEscritura.o Logger.o SignalHandler.o Random.o LockFile.o Output.o LogMessage.o Semaforo.o Caja.o

EJEC = Lanzador Administrador Calesita Kid Recaudador FilaCalesita FilaBoleto Fantasma

#all: de que depende, que tiene que existir o no haber sido modificado... 
all: $(EJEC) cleanO

Lanzador: Lanzador.o $(OBJ)
# $< es la primer dependencia, en este caso Lanzador.cpp
# $@ es el nombre de la regla en este caso Lanzador
	$(CC) $(CFLAGS) $< $(OBJ) -o $@ $(INC_PARAMS)

Administrador: Administrador.o $(OBJ)
	$(CC) $(CFLAGS) $< $(OBJ) -o $@ $(INC_PARAMS)

Calesita: Calesita.o $(OBJ)
	$(CC) $(CFLAGS) $< $(OBJ) -o $@ $(INC_PARAMS)

Kid: Kid.o $(OBJ)
	$(CC) $(CFLAGS) $< $(OBJ) -o $@ $(INC_PARAMS)

Recaudador: Recaudador.o $(OBJ)
	$(CC) $(CFLAGS) $< $(OBJ) -o $@ $(INC_PARAMS)

FilaCalesita: FilaCalesita.o $(OBJ)
	$(CC) $(CFLAGS) $< $(OBJ) -o $@ $(INC_PARAMS)

FilaBoleto: FilaBoleto.o $(OBJ)
	$(CC) $(CFLAGS) $< $(OBJ) -o $@ $(INC_PARAMS)

Fantasma: Fantasma.o $(OBJ)
	$(CC) $(CFLAGS) $< $(OBJ) -o $@ $(INC_PARAMS)

Lanzador.o: LanzadorProcesos.cpp $(DEPS_LANZADOR)
	$(CC) $(CFLAGS) -c $< -o $@ $(INC_PARAMS) -DLANZADOR

Administrador.o: Administrador.cpp $(DEPS_ADMINISTRADOR)
	$(CC) $(CFLAGS) -c $< -o $@ $(INC_PARAMS)  -DADMIN

Calesita.o: Calesita.cpp $(DEPS_CALESITA)
	$(CC) $(CFLAGS) -c $< -o $@ $(INC_PARAMS)  -DCALESITA 

Kid.o: Kid.cpp $(DEPS_KID)
	$(CC) $(CFLAGS) -c $< -o $@ $(INC_PARAMS)  -DKID

Recaudador.o: Recaudador.cpp $(DEPS_RECAUDADOR)
	$(CC) $(CFLAGS) -c $< -o $@ $(INC_PARAMS)  -DRECAUDADOR

FilaCalesita.o: FilaCalesita.cpp $(DEPS_FILACALESITA)
	$(CC) $(CFLAGS) -c $< -o $@ $(INC_PARAMS)  -DFILA2

FilaBoleto.o: FilaBoleto.cpp $(DEPS_FILABOLETO)
	$(CC) $(CFLAGS) -c $< -o $@ $(INC_PARAMS)  -DFILA1

Fantasma.o: Fantasma.cpp $(DEPS_FANTASMA)
	$(CC) $(CFLAGS) -c $< -o $@ $(INC_PARAMS) -DFANTASMA

Pipe.o: ./Pipes_y_Fifos/Pipe.cpp $(PIPE)
	$(CC) $(CFLAGS) -c $< -o $@ $(INC_PARAMS)

FifoLectura.o: ./Pipes_y_Fifos/FifoLectura.cpp Fifo.o $(FIFORD)
	$(CC) $(CFLAGS) -c $< -o $@ $(INC_PARAMS)

FifoEscritura.o: ./Pipes_y_Fifos/FifoEscritura.cpp Fifo.o $(FIFOWR) 
	$(CC) $(CFLAGS) -c $< -o $@ $(INC_PARAMS)

Fifo.o: ./Pipes_y_Fifos/Fifo.cpp ./Pipes_y_Fifos/Fifo.h
	$(CC) $(CFLAGS) -c $< -o $@ $(INC_PARAMS)

SignalHandler.o: ./Seniales/SignalHandler.cpp $(SIG) ./Seniales/EventHandler.h ./Seniales/SIGINT_Handler.h
	$(CC) $(CFLAGS) -c $< -o $@ $(INC_PARAMS)

LockFile.o: ./Locks/LockFile.cpp ./Locks/LockFile.h
	$(CC) $(CFLAGS) -c $< -o $@ $(INC_PARAMS)

Semaforo.o: ./Semaforos/Semaforo.cpp ./Semaforos/Semaforo.h
	$(CC) $(CFLAGS) -c $< -o $@ $(INC_PARAMS)

Random.o: ./utils/Random.cpp ./utils/Random.hpp
	$(CC) $(CFLAGS) -c $< -o $@ $(INC_PARAMS)

Caja.o: ./Caja.cpp ./Caja.h
	$(CC) $(CFLAGS) -c $< -o $@ $(INC_PARAMS)
	
Logger.o: ./logger/Logger.cpp ./logger/Output.hpp
	$(CC) $(CFLAGS) -c $< -o $@ $(INC_PARAMS)

Output.o: ./logger/Output.cpp ./logger/LogMessage.hpp 
	$(CC) $(CFLAGS) -c $< -o $@ $(INC_PARAMS)

LogMessage.o: ./logger/LogMessage.cpp ./logger/Info.hpp
	$(CC) $(CFLAGS) -c $< -o $@ $(INC_PARAMS)


.PHONY: cleanO clean

cleanO:
	rm *.o

clean:
	rm $(EJEC)
