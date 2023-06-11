#include "xerrori.h"
#include <search.h>

#define Num_elem 1000000

// IMPORTANTE le letture possono essere fatte in contemporanea


ENTRY *crea_entry(char *s){
	ENTRY *e = malloc(sizeof(ENTRY));
	int n = 1;
	if(e == NULL) termina("errore di allocazione entry");
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

// ok uso una pipe per ricevere dai threads

void tabella_hash(void){
	printf("tabella hash sta partendo");
	int fd = open("scrittori_to_hash", O_RDONLY);
	if(fd < 0) xtermina("Errore apertura pipe", __LINE__, __FILE__);

	printf("HASH: aperta pipe");
	char **stringhe = malloc(sizeof(char *)*100);
	if(stringhe == NULL) termina("errore allocazione");
	int dimensione = 100;
	int inseriti = 0;
	while(true){
		if(dimensione == inseriti){
			dimensione = dimensione*2;
			stringhe = realloc(stringhe, dimensione*sizeof(char));
			if(stringhe == NULL) termina("errore allocazione");
		}
		char * stringa;
		ssize_t e = read(fd, &stringa, sizeof(stringa));
		if(e != sizeof(stringa)) xtermina("Errore lettura pipe", __LINE__, __FILE__);
		//stringhe[inseriti] = strdup(stringa);
		inseriti++;
		if(e == 0) break;
		printf("HASH: Ricevuto: %s\n", stringa);
	}

	printf("HASH: termine ricezione");
	xclose(fd, __LINE__, __FILE__);
	printf("lettura finita.");
	
	// qui chiama aggiungi su metà delle stringhe
	// e poi conta sull'altra metà
}


int main(int argc, char *argv[]){
	printf("facendo partire la tabella hash");
	tabella_hash();
}
