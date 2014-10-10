#ifndef EVENTHANDLER_H_
#define EVENTHANDLER_H_

class EventHandler {

public:
	//solo lo uso, para señales que indiquen cierre del programa -> todas comparten este atributo
	sig_atomic_t gracefulQuit;

	EventHandler () : gracefulQuit(0)  {};
	virtual int handleSignal ( int signum ) = 0;
	virtual ~EventHandler () {};
};

#endif /* EVENTHANDLER_H_ */
