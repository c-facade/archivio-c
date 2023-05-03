
#include <stdio.h>
#include <stdlib.h>
#include <search.h>
#include <strings.h>
#include <string.h>

#define TABLE_SIZE 100

// Questo programma legge stringhe da linea di comando finchè non riceve
// la stringa "fine".
// poi stampa la tabella hash ottenuta
// permette di cercare una stringa
// quando si scrive la stringa "fine" termina.
int main(int argc, char **argv){
	
	int hashTable = hcreate(TABLE_SIZE);
	if(hashTable == 0){
		puts("Errore creazione hashtable");
		exit(1);
	}	
	
	char ** data = malloc(TABLE_SIZE*sizeof(char *));
	for(int i = 0; i< TABLE_SIZE; i++){
		data[i] = malloc(sizeof(char)*11);
		if(data[i] == NULL) exit(1);
	}
	if(data == NULL) exit(1);
	printf("Inserire stringhe, scrivere \"fine\" per terminare\n");
	scanf("%10[^\n]", data[0]);
	for(int i = 0; i< TABLE_SIZE; i++){
		if(strcasecmp(data[i], "fine") == 0){
			break;
		}
		ENTRY temp;
		temp.key = data[i];
		temp.data = data[i];
		if(hsearch(temp, ENTER) == NULL){
			fprintf(stderr, "entry failed\n");
			exit(1);
		}
		scanf(" %10[^\n]", data[i+1]);
	}


	char s[11];
	printf("Cerca una stringa(\"fine\" per terminare):\n");
	scanf(" %10[^\n]", s);
	while(strcasecmp(s, "fine")!= 0){
		ENTRY temp;
		temp.key=s;
		temp.data=s;
		ENTRY *result = hsearch(temp, FIND);
		if(result != NULL){
			printf("Trovata!\n");
		}
		else{
			printf("Non trovata.\n");
		}
		scanf(" %10[^\n]", s);
	}

	hdestroy();
	for(int i = 0; i<100; i++){
		free(data[i]);
	}
	free(data);
	return 0;
}
		

