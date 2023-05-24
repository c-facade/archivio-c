#include "xerrori.h"

#define Caposcrittore "caposc"

// iniziamo con una versione molto semplice in cui il
// capo scrittore invia le stringhe e gli scrittori le leggono


void caposcrittore(){
	int cs = open(Caposcrittore, O_RDONLY);
	if(cs < 0) xtermina("Errore apertura caposcrittore", __LINE__, __FILE__);
	while(true){
		int len;
		ssize_t e = read(cs, &len, sizeof(len));
		if(e == 0) break;
		// printf("Lunghexxa: %d\n", len);
		char * linea = malloc(sizeof(char)*(len+1));
		// in realtà qui ci dovrò mettere un termina
		// speciale per i thread
		if(linea == NULL) termina("Spazio esaurito");
		for(int i = 0; i<len; i++){
			e = read(cs, &(linea[i]), sizeof(char));
			if(e == 0) exit(1);
		}
		linea[len] = '\0';
		// printf("Stringa: %s\n", linea);
		char *saveptr;
		char * token = strtok_r(linea, ".,:; \n\r\t", &saveptr);
		while( token != NULL){
			printf("token: %s\n", token);
			token = strtok_r(NULL, ".,:; \n\r\t", &saveptr);
		}
	}
	printf("Fine\n");
}



int main(void){
	caposcrittore();
}
