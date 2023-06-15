#include "hash.h"

typedef struct {
	int numero_scrittori;
	rwsync *sync;
	int *stringhe_uniche;
} dati_caposcrittore;

typedef struct {
	int numero_lettori;
	rwsync *sync;
} dati_capolettore;

void *caposcrittore(void *args);

void *capolettore(void *args);
