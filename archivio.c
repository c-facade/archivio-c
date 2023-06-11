#include "xerrori.h"
#include "lettoriescrittori.h"

// questo file contiene il main
// chiama il capolettore, che ha il ruolo di comunicare con il
// server, e inviare i dati ai singoli thread scrittori


#define Num_elem 1000000

int main(void){
	int hashTable = hcreate(Num_elem);
	if(hashTable == 0){
		termina("Errore creazione tabella hash.");
	}

	rwsync sync;
	init_rwsync(&sync);

	int stringhe_uniche = 0;

	caposcrittore(3, &sync, &stringhe_uniche);
	capolettore(3, &sync);

}


