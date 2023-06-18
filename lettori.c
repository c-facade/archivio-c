#include "xerrori.h"
#include "lettoriescrittori.h"

#define Capolettore "capolet"
#define PC_buffer_len 10

// Facciamo la stessa prova che con gli scrittori
// con l'aggiunta che si scrive sul file lettori.log

typedef struct {
	int *index;
	char **buffer;
	FILE *f;
	sem_t *sem_free_slots;
	sem_t *sem_data_items;
	pthread_mutex_t *buffer_access;
	pthread_mutex_t *file_access;
	rwsync *sync;
} dati_lettore;


// corpo di un lettore
void *lbody(void *args){
	dati_lettore *a = (dati_lettore *) args;

	while(true){
		xsem_wait(a->sem_data_items, __LINE__, __FILE__);
		xpthread_mutex_lock(a->buffer_access, __LINE__, __FILE__);
			char *s = a->buffer[*(a->index)%PC_buffer_len];
			*(a->index) = *(a->index) + 1;
		xpthread_mutex_unlock(a->buffer_access, __LINE__, __FILE__);
		xsem_post(a->sem_free_slots, __LINE__, __FILE__);
		// il puntatore a NULL è il segnale di terminazione
		if(s == NULL) break;
		// stampo il dato che ho ricevuto
		printf("Lettore %d : %s\n", gettid(), s);
		int occorrenze = conta(s, a->sync);
		xpthread_mutex_lock(a->file_access, __LINE__, __FILE__);
		fprintf(a->f, "%s %d\n", s, occorrenze);
		xpthread_mutex_unlock(a->file_access, __LINE__, __FILE__);
		free(s);
	}

	printf("Thread lettore %d sta terminando\n", gettid());
	return NULL;
}

void *capolettore(void *args){

	dati_capolettore *dati = (dati_capolettore *) args;
	rwsync * sync = dati->sync;
	
	// apro il file di logging
	
	FILE *f = xfopen("lettori.log", "w", __LINE__, __FILE__);

	// inizializzazione semafori e mutex
	
	sem_t sem_free_slots, sem_data_items;
	xsem_init( &sem_free_slots, 0, PC_buffer_len, __LINE__, __FILE__);
	xsem_init( &sem_data_items, 0, 0, __LINE__, __FILE__);

	pthread_mutex_t buffer_access = PTHREAD_MUTEX_INITIALIZER;
	pthread_mutex_t file_access = PTHREAD_MUTEX_INITIALIZER;

	// inizializzo buffer e indici
	
	int pindex = 0;
	int cindex = 0;
	char *buffer[PC_buffer_len];

	// creo i dati per i thread
	dati_lettore a[dati->numero_lettori];
	for(int i = 0; i < dati->numero_lettori; i++){
		a[i].index = &cindex;
		a[i].buffer = buffer;
		a[i].sem_free_slots = &sem_free_slots;
		a[i].sem_data_items = &sem_data_items;
		a[i].f = f;
		a[i].buffer_access = &buffer_access;
		a[i].file_access = &file_access;
		a[i].sync = sync;
	}

	// creo e faccio partire i thread
	
	pthread_t t[dati->numero_lettori];

	for(int i = 0; i<dati->numero_lettori; i++){
		xpthread_create(&t[i], NULL, &lbody, &a[i], __LINE__, __FILE__);
	}

	int cl = open(Capolettore, O_RDONLY);
	// anche qui bisogna usare un termina diverso
	if(cl < 0) xthread_termina("Errore apertura capolettore", __LINE__, __FILE__);
	
	// leggo dalla fifo le stringhe da cercare
	// e le invio su per il buffer ai lettori
	
	while(true){
		
		printf("Capolettore: leggendo da fifo\n");
		
		int len;
		ssize_t e = read(cl, &len, sizeof(len));
		

		// printf("Capolettore -- Lunghezza: %d\n", len);
		if(e == 0){
			printf("e == 0\n");
			break;
		}
		
		
		char * linea = malloc(sizeof(char)*(len+1));
		
		if(linea == NULL) termina("Spazio esaurito");
		for(int i = 0; i<len; i++){
			e = read(cl, &(linea[i]), sizeof(char));
			if(e == 0) break;
		}

		linea[len] = '\0';

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
			
			// prendo un nuovo token
			token = strtok_r(NULL, ".,:; \n\t\r", &saveptr);
		}

		free(linea);
	}
	
	// mando i segnali di terminazione
	for(int i = 0; i<dati->numero_lettori; i++){
		xsem_wait(&sem_free_slots, __LINE__, __FILE__);
		xpthread_mutex_lock(&buffer_access, __LINE__, __FILE__);
		buffer[pindex%PC_buffer_len] = NULL;
		pindex++;
		xpthread_mutex_unlock(&buffer_access, __LINE__, __FILE__);
		xsem_post(&sem_data_items, __LINE__, __FILE__);
	}

	// aspetto che i thread terminino
	
	for(int i = 0; i<dati->numero_lettori; i++){
		xpthread_join(t[i], NULL, __LINE__, __FILE__);
	}
	xpthread_mutex_destroy(&buffer_access, __LINE__, __FILE__);
	xpthread_mutex_destroy(&file_access, __LINE__, __FILE__);

	close(cl);
	fclose(f);
	printf("Terminazione capo lettore.\n");
	return NULL;
}
