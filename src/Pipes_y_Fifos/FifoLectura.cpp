#include "FifoLectura.h"

#include<iostream>

FifoLectura::FifoLectura(const std::string nombre) : Fifo(nombre) {
}

FifoLectura::~FifoLectura() {
}

void FifoLectura::abrir() {
	std::cout << "abro" << nombre.c_str() << std::endl;
	fd = open ( nombre.c_str(), O_RDONLY );
	std::cout << "abri" << fd << std::endl;

}

ssize_t FifoLectura::leer(void* buffer,const ssize_t buffsize) const {
	return read ( fd,buffer,buffsize );
}
