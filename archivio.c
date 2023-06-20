#include "lettoriescrittori.h"

// questo file contiene il main e il corpo del signal handler
// il main chiama il capolettore e il caposcrittore
// e il thread gestore di segnali

#define Num_elem 1000000


// dati del gestore segnai
typedef struct{
	int * stringhe_uniche;
	bool continua;
} dati_gs;


// corpo del gestore di segnali
void *gs_body(void *args);


int main(int argc, char **argv){
	if(argc < 3){
		printf("Uso: %s numero_lettori numero_scrittori", argv[0]);
		exit(1);
	}

	// blocca i segnali in modo che sia
	// il thread gestore a gestirli
	sigset_t mask;
	sigfillset(&mask);
	pthread_sigmask(SIG_BLOCK, &mask, NULL);
	
	int stringhe_uniche = 0;
	
	// dati del gestore di segnali
	dati_gs a;
	a.stringhe_uniche = &stringhe_uniche;
	a.continua = true;

	pthread_t gestore_segnali;
	xpthread_create(&gestore_segnali, NULL, &gs_body, &a, __LINE__, __FILE__);

	int r = atoi(argv[1]);
	int w = atoi(argv[2]);
	assert(r > 0 && r < 50 && w > 0 && w < 50);
	
	// creazione tabella hash	
	int hashTable = hcreate(Num_elem);
	if(hashTable == 0){
		termina("Errore creazione tabella hash.");
	}

	// inizializzazione struct di sincronizzazione tra scrittori e lettori
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

	// entro in un loop apparentemente infinito
	// che termina quando il gestore di segnali riceve
	// un SIGTERM
	do {
		sleep(0.5);
	} while(a.continua);

	// ho ricevuto SIGTERM
	// capolettore e caposcrittore terminano perchè
	// il server ha già chiuso le pipe
	xpthread_join(tcs, NULL, __LINE__, __FILE__);
	xpthread_join(tcl, NULL, __LINE__, __FILE__);
	xpthread_join(gestore_segnali, NULL, __LINE__, __FILE__);
	
	//printf("Terminazione archivio.\nStringhe uniche: %d\n", stringhe_uniche);
	printf("%d\n", stringhe_uniche);
	return 0;
}


// corpo del gestore di segnali
void *gs_body(void *args) {
	dati_gs *a = (dati_gs *) args;
	//printf("Partenza gestore segnali\n");
	
	sigset_t mask;
	sigfillset(&mask);
	int s;
	while(true) {
		int e = sigwait(&mask, &s);
		if(e != 0) perror("Errore sigwait");
		if(s == SIGINT){
			char buffer[12];
			int numero = *(a->stringhe_uniche);
			// conversione in stringa
			int i = 0;
			do{
				buffer[i] = numero % 10 + '0';
				i++;
				numero = numero / 10;
			} while(numero > 0 && i < 11);
			buffer[i] = '\n';
			int len = i+1;
			i--;
			for(int j = 0; j<i; j++, i--){
				char c = buffer[i];
				buffer[i] = buffer[j];
				buffer[j] = c;
			}
			// e stampo la stringa ottenuta
			write(STDOUT_FILENO, buffer, len);
		}
		if(s == SIGTERM){
			a->continua = false;
			break;
		}
	}
	return NULL;
}
