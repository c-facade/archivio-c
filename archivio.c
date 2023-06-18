#include "lettoriescrittori.h"

// questo file contiene il main
// chiama il capolettore, che ha il ruolo di comunicare con il
// server, e inviare i dati ai singoli thread scrittori


#define Num_elem 1000000

typedef struct{
	int * stringhe_uniche;
	bool continua;
} dati_segnali;



void *segnali_body(void *args) {
	// qui ci sarà stringhe uniche e che altro?
	dati_segnali *a = (dati_segnali *) args;
	printf("Partenza gestore segnali\n");

	sigset_t mask;
	sigfillset(&mask);
	//sigdelset(&mask, SIGINT);
	//sigdelset(&mask, SIGTERM);
	int s;
	while(true) {
		int e = sigwait(&mask, &s);
		if(e != 0) perror("Errore sigwait");
		if(s == SIGINT){
			printf("Stringhe uniche: %d\n", *(a->stringhe_uniche));
		}
		if(s == SIGTERM){
			a->continua = false;
			return NULL;
		}
	}
}



int main(int argc, char **argv){
	if(argc < 3){
		printf("Uso: %s numero_lettori numero_scrittori.", argv[0]);
		exit(1);
	}

	sigset_t mask;
	sigfillset(&mask);
	pthread_sigmask(SIG_BLOCK, &mask, NULL);
	
	int stringhe_uniche = 0;
	
	dati_segnali a;
	a.stringhe_uniche = &stringhe_uniche;
	a.continua = true;

	pthread_t gestore_segnali;
	xpthread_create(&gestore_segnali, NULL, &segnali_body, &a, __LINE__, __FILE__);

	int r = atoi(argv[1]);
	int w = atoi(argv[2]);
	int hashTable = hcreate(Num_elem);
	if(hashTable == 0){
		termina("Errore creazione tabella hash.");
	}

	rwsync sync;
	init_rwsync(&sync);


	dati_caposcrittore dati_cs;
	dati_capolettore dati_cl;
	dati_cs.sync = &sync;
	dati_cl.sync = &sync;
	dati_cs.numero_scrittori = w;
	dati_cl.numero_lettori = r;
	dati_cs.stringhe_uniche = &stringhe_uniche;

	pthread_t tcl, tcs;
	
	xpthread_create(&tcs, NULL, &caposcrittore, &dati_cs, __LINE__, __FILE__);
	
	xpthread_create(&tcl, NULL, &capolettore, &dati_cl, __LINE__, __FILE__);


	do {
		sleep(0.5);
	} while(a.continua);

	xpthread_join(tcs, NULL, __LINE__, __FILE__);
	xpthread_join(tcl, NULL, __LINE__, __FILE__);
	xpthread_join(gestore_segnali, NULL, __LINE__, __FILE__);
	
	printf("Terminazione archivio.\nStringhe uniche: %d\n", stringhe_uniche);
	return 0;
}


