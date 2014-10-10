
CC = g++
CFLAGS = -Wall -Wpedantic

PREPR = -DLANZADOR -DKID -DFILA1 -DFILA2 -DADMIN -DRECAUDADOR -DCALESITA -DFANTASMA

INC = $(.) $(./Locks) $(./logger) $(./Memoria_Compartida) $(./Pipes_y_Fifos) $(./Seniales) $(./utils)
INC_PARAMS = $(foreach d, $(INC), -I$d)

MEM = ./Memoria_Compartida/MemoriaCompartida.h
PIPE = ./Pipes_y_Fifos/Pipe.h
CONST = Constantes.h
LOG = ./logger/Logger.hpp
UTILS = ./utils/Utils.hpp
FIFORD = ./Pipes_y_Fifos/FifoLectura.h
FIFOWR = ./Pipes_y_Fifos/FifoEscritura.h
SIG = ./Seniales/SignalHandler.h
LOCKRD = ./Locks/LockRead.hpp
LOCKWR = ./Locks/LockWrite.hpp

DEPS_LANZADOR = $(MEM) $(PIPE) $(CONST) $(LOG) $(UTILS)
DEPS_ADMINISTRADOR = $(CONST) $(LOG) $(UTILS) $(MEM) $(LOCKRD)
DEPS_CALESITA = $(CONST) $(LOG) $(UTILS) $(MEM) $(LOCKWR) $(LOCKRD)
DEPS_KID = $(CONST) $(LOG) $(UTILS) $(MEM) $(PIPE) $(FIFORD) $(LOCKWR) $(LOCKRD) 
DEPS_RECAUDADOR = $(MEM) $(FIFORD) $(LOCKWR) $(LOCKRD) $(CONST) $(LOG) $(UTILS)
DEPS_FILACALESITA = $(CONST) $(LOG) $(UTILS) $(SIG) $(FIFOWR) $(PIPE)
DEPS_FILABOLETO = $(CONST) $(LOG) $(UTILS) $(SIG) $(FIFOWR) $(PIPE)
DEPS_FANTASMA = $(CONST) $(LOG) $(UTILS) $(LOCKWR) $(MEM)

OBJ = Pipe.o Fifo.o FifoLectura.o FifoEscritura.o Logger.o SignalHandler.o  Random.o LockFile.o Output.o LogMessage.o

#all: de que depende, que tiene que existir o no haber sido modificado... 
all: Lanzador Administrador Calesita Kid Recaudador FilaCalesita FilaBoleto Fantasma clean

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
	$(CC) $(CFLAGS) -c $< -o $@ $(INC_PARAMS) $(PREPR)

Administrador.o: Administrador.cpp $(DEPS_ADMINISTRADOR)
	$(CC) $(CFLAGS) -c $< -o $@ $(INC_PARAMS) $(PREPR)

Calesita.o: Calesita.cpp $(DEPS_CALESITA)
	$(CC) $(CFLAGS) -c $< -o $@ $(INC_PARAMS) $(PREPR)

Kid.o: Kid2.cpp $(DEPS_KID)
	$(CC) $(CFLAGS) -c $< -o $@ $(INC_PARAMS) $(PREPR)

Recaudador.o: Recaudador.cpp $(DEPS_RECAUDADOR)
	$(CC) $(CFLAGS) -c $< -o $@ $(INC_PARAMS) $(PREPR)

FilaCalesita.o: FilaCalesita.cpp $(DEPS_FILACALESITA)
	$(CC) $(CFLAGS) -c $< -o $@ $(INC_PARAMS) $(PREPR)

FilaBoleto.o: FilaBoleto.cpp $(DEPS_FILABOLETO)
	$(CC) $(CFLAGS) -c $< -o $@ $(INC_PARAMS) $(PREPR)

Fantasma.o: Fantasma.cpp $(DEPS_FANTASMA)
	$(CC) $(CFLAGS) -c $< -o $@ $(INC_PARAMS) $(PREPR)

Pipe.o: ./Pipes_y_Fifos/Pipe.cpp $(PIPE)
	$(CC) $(CFLAGS) -c $< -o $@ $(INC_PARAMS)

FifoLectura.o: ./Pipes_y_Fifos/FifoLectura.cpp Fifo.o $(FIFORD)
	$(CC) $(CFLAGS) -c $< -o $@ $(INC_PARAMS)

FifoEscritura.o: ./Pipes_y_Fifos/FifoEscritura.cpp Fifo.o $(FIFOWR) 
	$(CC) $(CFLAGS) -c $< -o $@ $(INC_PARAMS)

Fifo.o: ./Pipes_y_Fifos/Fifo.cpp ./Pipes_y_Fifos/Fifo.h
	$(CC) $(CFLAGS) -c $< -o $@ $(INC_PARAMS)

SignalHandler.o: ./Seniales/SignalHandler.cpp $(SIG) ./Seniales/EventHandler.h ./Seniales/SIGUSR1_Handler.h
	$(CC) $(CFLAGS) -c $< -o $@ $(INC_PARAMS)

LockFile.o: ./Locks/LockFile.cpp ./Locks/LockFile.h
	$(CC) $(CFLAGS) -c $< -o $@ $(INC_PARAMS)

Random.o: ./utils/Random.cpp ./utils/Random.hpp
	$(CC) $(CFLAGS) -c $< -o $@ $(INC_PARAMS)

Logger.o: ./logger/Logger.cpp ./logger/Output.hpp
	$(CC) $(CFLAGS) -c $< -o $@ $(INC_PARAMS)

Output.o: ./logger/Output.cpp ./logger/LogMessage.hpp 
	$(CC) $(CFLAGS) -c $< -o $@ $(INC_PARAMS)

LogMessage.o: ./logger/LogMessage.cpp ./logger/Info.hpp
	$(CC) $(CFLAGS) -c $< -o $@ $(INC_PARAMS)


.PHONY: clean

clean:
	rm *.o
