#include "hash.h"

void caposcrittore(int numero_scrittori, rwsync *sync, int *stringhe_uniche);

void capolettore(int numero_lettori, rwsync *sync);
