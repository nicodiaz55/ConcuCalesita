#ifndef SIGINT_HANDLER_H_
#define SIGINT_HANDLER_H_

#include <signal.h>
#include <assert.h>

#include "../Constantes.h"
#include "EventHandler.h"

class SIGINT_Handler : public EventHandler {

	public:

		SIGINT_Handler () : EventHandler() {
		}

		~SIGINT_Handler () {
		}

		virtual int handleSignal ( int signum ) {
			assert ( signum == SIGINT );
			this->gracefulQuit = CERRAR;
			return 0;
		}

		sig_atomic_t getGracefulQuit () const {
			return this->gracefulQuit;
		}

};

#endif /* SIGINT_HANDLER_H_ */
