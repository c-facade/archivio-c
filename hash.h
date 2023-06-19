#include "xerrori.h"
#include <search.h>

#define Num_elem 1000000

typedef struct{
	int readers;
	bool writing;
	pthread_cond_t cond;
	pthread_mutex_t mutex;
} rwsync;


// inizializza la struct di sincronizzazione
void init_rwsync(rwsync *sync);

ENTRY *crea_entry(char *s);

void distruggi_entry(ENTRY *e);

void aggiungi(char *s, int *stringhe_uniche, rwsync *sync);

int conta(char *s, rwsync *sync);
