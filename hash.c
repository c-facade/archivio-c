#include "hash.h"

void init_rwsync(rwsync *sync)
{
	sync->readers = 0;
	sync->writing = false;
	xpthread_cond_init(&sync->cond, NULL, __LINE__, __FILE__);
	xpthread_mutex_init(&sync->mutex, NULL, __LINE__, __FILE__);
}

void xthread_termina(char *msg, int linea, char *file){
	fprintf(stderr, "%s == %d == Linea: %d, File: %s\n", msg, getpid(), linea, file);
	pthread_exit(NULL);
}

ENTRY *crea_entry(char *s){
	ENTRY *e = malloc(sizeof(ENTRY));
	int n = 1;
	if(e == NULL){
		termina("errore di allocazione entry");
	}
	e->key = strdup(s);
	e->data = (int *) malloc(sizeof(int));
	if(e->key == NULL || e->data == NULL){
		termina("errore di allocazione entry");
	}
	*((int *)e->data) = n;
	return e;
}

void distruggi_entry(ENTRY *e){
	free(e->key);
	free(e->data);
	free(e);
}

void aggiungi(char *s, int *stringhe_uniche, rwsync *sync){
	ENTRY *new = crea_entry(s);
	// inizio scrittura in tabella

	pthread_mutex_lock(&sync->mutex);
	while(sync->writing || sync->readers>0)
		pthread_cond_wait(&sync->cond, &sync->mutex);
	sync->writing = true;
	pthread_mutex_unlock(&sync->mutex);

  // printf("Thread %d ha ottenuto il permesso di scrittura.\n", gettid());

	// ora si può scriver
	ENTRY *item = hsearch(*new, FIND);
	if(item == NULL){
		ENTRY *success = hsearch(*new, ENTER);
		// solo uno scrittore alla volta quindi non ci sono
		// problemi con questa variabile
		(*stringhe_uniche)++;
		if(success == NULL) xthread_termina("errore inserimento in tabella", __LINE__, __FILE__);
	}
	else{
		assert(strcmp(new->key, item->key) == 0);
		//printf("La stringa era già presente %d volte\n", *((int *) item->data));
		int *d = (int *) item->data;
		*d += 1;
		distruggi_entry(new);
	}

	// termine scrittura in tabella
	assert(sync->writing);
	pthread_mutex_lock(&sync->mutex);
	sync->writing = false;
	pthread_cond_broadcast(&sync->cond);
	pthread_mutex_unlock(&sync->mutex);

	//printf("Il thread %d ha rilasciato l'accesso in scrittura.\n", gettid());
}

int conta(char *s, rwsync *sync){
	ENTRY *item = crea_entry(s);

	// otteniamo il via libera per leggere
	pthread_mutex_lock(&sync->mutex);
	while(sync->writing == true)
		pthread_cond_wait(&sync->cond, &sync->mutex);
	sync->readers++;
	pthread_mutex_unlock(&sync->mutex);
	
	//printf("Il thread %d ha ottenuto l'accesso in lettura\n", gettid());
	int occorrenze;
	// inizio lettura della tabella
	ENTRY *trovata = hsearch(*item, FIND);
	if(trovata == NULL){
		occorrenze = 0;
		//printf("%s -> non trovato\n", s);
	}
	else{
		occorrenze = *((int*) trovata->data);
		//printf("%s -> Occorrenze: %d\n", s, occorrenze);
	}
	// termine lettura in tabella
	assert(sync->readers > 0);
	assert(!sync->writing);
	pthread_mutex_lock(&sync->mutex);
	sync->readers--;
	if(sync->readers == 0)
		pthread_cond_signal(&sync->cond); // segnalando ad un solo writer
	pthread_mutex_unlock(&sync->mutex);

	//printf("Il thread %d ha rilasciato l'accesso in lettura\n", gettid());
	distruggi_entry(item);
	return occorrenze;
}


