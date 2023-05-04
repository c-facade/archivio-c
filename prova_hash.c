
#include <stdio.h>
#include <stdlib.h>
#include <search.h>
#include <strings.h>
#include <string.h>

#define TABLE_SIZE 100
int numeri[100];

void aggiungi(char *s, int *stringhe_uniche){
	// se la stringa
	// non è nella tabella hash deve essere inserita
	// con valore == 1
	ENTRY new_item;
	new_item.key = s;
	new_item.data = &(numeri[1]);
	ENTRY *item = hsearch(new_item, FIND);
	if(item == NULL){
		ENTRY *success = hsearch(new_item, ENTER);
		(*stringhe_uniche)++;
		if(success == NULL) exit(1);
		return;
	}
	//se invece è presente il valore corrente va incrementato
	int *occ = item->data;
	item->data = &(numeri[(*occ)+1]);

}

int conta(char *s){
	ENTRY item;
	item.key = s;
	item.data = NULL;
	ENTRY *found = hsearch(item, FIND);
	if(found == NULL){
		return 0;
	}
	int *occ = found->data;
	return *occ;
}

	// Questo programma legge stringhe da linea di comando finchè non riceve
// la stringa "fine".
// permette di cercare una stringa
// dice se è stata trovata e con quante occorrenze
// quando si scrive la stringa "fine" termina.
int main(int argc, char **argv){
	
	int hashTable = hcreate(TABLE_SIZE);
	if(hashTable == 0){
		puts("Errore creazione hashtable");
		exit(1);
	}

	int stringhe_uniche = 0;

	for(int i = 0; i < 100; i++){
		numeri[i] = i;
	}
	char ** stringhe = malloc(TABLE_SIZE*sizeof(char *));
	if(stringhe == NULL) exit(1);
	for(int i = 0; i< TABLE_SIZE; i++){
		stringhe[i] = malloc(sizeof(char)*11);
		if(stringhe[i] == NULL) exit(1);
	}
	printf("Inserire stringhe, scrivere \"fine\" per terminare\n");
	scanf("%10[^\n]", stringhe[0]);
	for(int i = 0; i< TABLE_SIZE; i++){
		if(strcasecmp(stringhe[i], "fine") == 0){
			break;
		}
		aggiungi(stringhe[i], &stringhe_uniche);
		scanf(" %10[^\n]", stringhe[i+1]);
	}


	char s[11];
	printf("Cerca una stringa(\"fine\" per terminare):\n");
	scanf(" %10[^\n]", s);
	while(strcasecmp(s, "fine")!= 0){
		int occ;
		occ = conta(s);
		if(occ != 0){
			printf("Trovata %d volte\n", occ);
		}
		else{
			printf("non trovata\n");
		}
		scanf(" %10[^\n]", s);
	}
	printf("Stringhe uniche: %d\n", stringhe_uniche);
	hdestroy();
	for(int i = 0; i<100; i++){
		free(stringhe[i]);
	}
	free(stringhe);
	return 0;
}
		

