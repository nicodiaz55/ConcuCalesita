#include "FifoLectura.h"

#include<iostream>

FifoLectura::FifoLectura(const std::string nombre) : Fifo(nombre) {
}

FifoLectura::~FifoLectura() {
}

int FifoLectura::abrir() {
	fd = open ( nombre.c_str(), O_RDONLY );
	return fd;
}

ssize_t FifoLectura::leer(void* buffer,const ssize_t buffsize) const {
	return read ( fd,buffer,buffsize );
}
