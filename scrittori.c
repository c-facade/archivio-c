#include "xerrori.h"
#include "lettoriescrittori.h"

#define Caposcrittore "caposc"
#define PC_buffer_len 10

// iniziamo con una versione molto semplice in cui il
// capo scrittore invia le stringhe e gli scrittori le leggono

// NOTE DAL FORUM
// Le condition variables vanno usate obbligatoriamente per gestire
// i lettori e scrittori concorrenti nel loro accesso alla
// tabella hash condivisa, secondo quello che abbiamo visto
// a lezione

typedef struct {
	int *index;
	char **buffer;
	sem_t *sem_free_slots;
	sem_t *sem_data_items;
	pthread_mutex_t *buffer_access;
	int *stringhe_uniche;
	rwsync *sync;
} dati_scrittore;


// corpo dello scrittore
void *sbody(void *args){
	dati_scrittore *a = (dati_scrittore *) args;
	
	printf("Inizio scrittore %d\n", gettid());
	while(true){
		xsem_wait(a->sem_data_items, __LINE__, __FILE__);
		xpthread_mutex_lock(a->buffer_access, __LINE__, __FILE__);
			char *s = a->buffer[*(a->index)%PC_buffer_len];
			*(a->index) = *(a->index) + 1;
		xpthread_mutex_unlock(a->buffer_access, __LINE__, __FILE__);
		xsem_post(a->sem_free_slots, __LINE__, __FILE__);
		// il segnale di terminazione è s = NULL
		if(s == NULL){
			printf("Lo scrittore %d ha ricevuto una stringa NULL e ha terminato.\n", gettid());
			break;
		}
		printf("%d : %s\n", gettid(), s);
		aggiungi(s, a->stringhe_uniche, a->sync);
		free(s);
	}

	printf("Thread scrittore %d sta terminando\n", gettid());
	return NULL;
}


void *caposcrittore(void *args){
	
	printf("Inizio caposcrittore\n");
	dati_caposcrittore *dati = (dati_caposcrittore *) args;
	rwsync *sync = dati->sync;
	// inizializzamo semafori e mutex
	
	sem_t sem_free_slots, sem_data_items;
	xsem_init( &sem_free_slots, 0, PC_buffer_len, __LINE__, __FILE__);
	xsem_init( &sem_data_items, 0, 0, __LINE__, __FILE__);

	pthread_mutex_t buffer_access = PTHREAD_MUTEX_INITIALIZER;

	// inizializzo buffer e indici
	int pindex = 0;
	int cindex = 0;
	char * buffer[PC_buffer_len];


//	if(e != 0) termina("Errore creazione named pipe");

	/*
	// creo i dati per i thread
	dati_scrittore a[dati->numero_scrittori];
	for(int i = 0; i< dati->numero_scrittori; i++){
		a[i].index = &cindex;
		a[i].buffer = buffer;
		a[i].sem_free_slots = &sem_free_slots;
		a[i].sem_data_items = &sem_data_items;
		a[i].buffer_access = &buffer_access;
		a[i].stringhe_uniche = dati->stringhe_uniche;
		a[i].sync = sync;
	}

	pthread_t t[dati->numero_scrittori];
	
	printf("Caposcrittore fa partire i thread scrittori\n");	
	for(int i = 0; i<dati->numero_scrittori; i++){
		xpthread_create(&t[i], NULL, &sbody, &a[i], __LINE__, __FILE__);
	}
	*/
	int cs = open(Caposcrittore, O_RDONLY);
	if(cs < 0) xtermina("Errore apertura pipe caposcrittore", __LINE__, __FILE__);
	while(true){
		printf("Caposcrittore: leggendo da fifo\n");
		int len;
		ssize_t e = read(cs, &len, sizeof(len));
		if(e == 0) break;
		printf("Lunghexxa: %d\n", len);
		char * linea = malloc(sizeof(char)*(len+1));
		// in realtà qui ci dovrò mettere un termina
		// speciale per i thread
		if(linea == NULL) xthread_termina("Spazio esaurito", __LINE__, __FILE__);
		for(int i = 0; i<len; i++){
			e = read(cs, &(linea[i]), sizeof(char));
			if(e == 0) exit(1);
		}
		linea[len] = '\0';
		printf("Caposcrittore -- Stringa: %s\n", linea);
		/*
		char *saveptr;
		char * token = strtok_r(linea, ".,:; \n\r\t", &saveptr);
		while( token != NULL){
			
			// ora mando su per il buffer il token
			char * newstring = strdup(token);
			xsem_wait(&sem_free_slots, __LINE__, __FILE__);
			xpthread_mutex_lock(&buffer_access, __LINE__, __FILE__);
			buffer[pindex%PC_buffer_len] = newstring;
			pindex++;
			xpthread_mutex_unlock(&buffer_access, __LINE__, __FILE__);
			xsem_post(&sem_data_items, __LINE__, __FILE__);
			token = strtok_r(NULL, ".,:; \n\r\t", &saveptr);
		}
		*/
		free(linea);
	}


	/*
	// mando i segnali di terminazione che sono un puntatore
	// a NULL
	for(int i = 0; i<dati->numero_scrittori; i++){
		xsem_wait(&sem_free_slots, __LINE__, __FILE__);
		xpthread_mutex_lock(&buffer_access, __LINE__, __FILE__);
		buffer[pindex%PC_buffer_len] = NULL;
		pindex++;
		xpthread_mutex_unlock(&buffer_access, __LINE__, __FILE__);
		xsem_post(&sem_data_items, __LINE__, __FILE__);
	}
	for(int i = 0; i<dati->numero_scrittori; i++){
		xpthread_join(t[i], NULL, __LINE__, __FILE__);
	}
	*/
	pthread_mutex_destroy(&buffer_access);
	
	close(cs);
	
	printf("Fine\n");
	
	return NULL;
}

