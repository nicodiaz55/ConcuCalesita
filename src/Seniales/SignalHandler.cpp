#include "SignalHandler.h"

SignalHandler* SignalHandler :: instance = NULL;
EventHandler* SignalHandler :: signal_handlers [ NSIG ];

SignalHandler :: SignalHandler () {
}

SignalHandler* SignalHandler :: getInstance () {

	if ( instance == NULL )
		instance = new SignalHandler ();

	return instance;
}

void SignalHandler :: destruir () {
	if ( instance != NULL ) {
		delete ( instance );
		instance = NULL;
	}
}

int SignalHandler :: registrarHandler ( int signum,EventHandler* eh ) {

	int res = 0;

	EventHandler* old_eh = SignalHandler :: signal_handlers [ signum ];
	SignalHandler :: signal_handlers [ signum ] = eh;

	struct sigaction sa;
	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = SignalHandler :: dispatcher;
	sigemptyset ( &sa.sa_mask );	// inicializa la mascara de seniales a bloquear durante la ejecucion del handler como vacio


	res = sigaddset ( &sa.sa_mask,signum );
	if (res == -1) { return RES_ERROR_SIGADDSET;}

	res = sigaction ( signum,&sa,0 );	// cambiar accion de la senial
	if (res == -1) { return RES_ERROR_SIGACTION;}

	eh = old_eh;

	return 0;
}

void SignalHandler :: dispatcher ( int signum ) {

	if ( SignalHandler :: signal_handlers [ signum ] != 0 )
		SignalHandler :: signal_handlers [ signum ]->handleSignal ( signum );
}

int SignalHandler :: removerHandler ( int signum ) {

	SignalHandler :: signal_handlers [ signum ] = NULL;
	return 0;
}
