#include "lettoriescrittori.h"

// questo file contiene il main
// chiama il capolettore, che ha il ruolo di comunicare con il
// server, e inviare i dati ai singoli thread scrittori


#define Num_elem 1000000

int main(int argc, char **argv){
	if(argc < 3){
		termina("Non abbastanza argomenti.");
	}
	int r = atoi(argv[1]);
	int w = atoi(argv[2]);
	int hashTable = hcreate(Num_elem);
	if(hashTable == 0){
		termina("Errore creazione tabella hash.");
	}

	rwsync sync;
	init_rwsync(&sync);

	int stringhe_uniche = 0;

	dati_caposcrittore dati_cs;
	dati_capolettore dati_cl;
	dati_cs.sync = &sync;
	dati_cl.sync = &sync;
	dati_cs.numero_scrittori = w;
	dati_cl.numero_lettori = r;
	dati_cs.stringhe_uniche = &stringhe_uniche;

	pthread_t tcl, tcs;
	xpthread_create(&tcl, NULL, &capolettore, &dati_cl, __LINE__, __FILE__);
	xpthread_create(&tcs, NULL, &caposcrittore, &dati_cs, __LINE__, __FILE__);

	xpthread_join(tcl, NULL, __LINE__, __FILE__);
	xpthread_join(tcs, NULL, __LINE__, __FILE__);
	
	printf("Terminazione.\n Stringhe uniche: %d\n", stringhe_uniche);
}


