#include "xerrori.h"

#define Num_elem 1000000
#define PC_buffer_len 10
#define QUI __LINE__,__FILE__

// da togliere
#include <search.h>
#include <time.h>

// nel progetto vero dovremmo dividere le funzioni per
// l'heap, gli scrittori e i consumatori in files diversi.

int stringhe_uniche = 0;

// da fare: impedire l'accesso simultaneo allo heap
// verificare quali sono effettivamente le sezioni critiche
// per il resto dovrebbe funzionare


void aggiungi(char *s){
	ENTRY new_item;
	new_item.key = s;
	int *zero = malloc(sizeof(int));
	*zero = 0;
	new_item.data = zero;
	ENTRY *item = hsearch(new_item, FIND);
	if(item == NULL){
		ENTRY *success = hsearch(new_item, ENTER);
		if(success == NULL) termina("Errore hashtable");
		stringhe_uniche++;
		return;
	}
	item->data += 1;
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



typedef struct {
	char **buffer;
	int *index;
	pthread_mutex_t *mutex;
	sem_t *sem_free_slots;
	sem_t *sem_data_items;
} dati_scrittore;

void lettore();

void caposcrittore(void *args){
	// riceve cose da una named pipe
}

void *scrittore(void *args){
	dati_scrittore *a = (dati_scrittore *) args;
	printf("Scrittore %d partito\n", gettid());
	while(true){
		char * stringa;
		xsem_wait(a->sem_data_items, QUI);
		{
			xpthread_mutex_lock(a->mutex, QUI);
			stringa = a->buffer[*(a->index) %PC_buffer_len];
			a->index++;
			if(stringa == NULL) return NULL;
			aggiungi(stringa);
			xpthread_mutex_unlock(a->mutex, QUI);
		}
		xsem_post(a->sem_free_slots, QUI);
	}
}


int main(int argc, char **argv){
	int hashTable = hcreate(Num_elem);
	if (hashTable == 0){
		termina("Errore creazione hashtable");
	}

	char ** stringhe = malloc(100*sizeof(char *));
	if(stringhe == NULL) termina("Errore allocazione");

	
	// dichiara e fa partire il capo scrittore
	
	//pthread_t capo_scrittore;
	//xpthread_create(capo_scrittore, NULL, *caposcrittore, &arg, QUI);
	pthread_t t[5];

	printf("Inserire stringhe, scrivere \"fine\" per terminare\n");
	stringhe[0] = malloc(11*sizeof(char));
	// bisogna sempre controllare se la malloc ha funzionato
	//if(stringhe[0] == NULL) termina("Errore alloc");
	scanf("%10[^\n]", stringhe[0]);
	int static string_num;
	for(int i = 0; i< Num_elem; i++){
		if(strcasecmp(stringhe[i], "fine") == 0){
			string_num = i;
			break;
		}
		stringhe[i+1] = malloc(11*sizeof(char));
		if(stringhe[i+1] == NULL) termina("Errore alloc");
		scanf(" %10[^\n]", stringhe[i+1]);
	}
	
	printf("Ora chiamo gli scrittori\n");

	//preparazione buffer

	int cindex, pindex = 0;
	// dichiarare due semafori
	sem_t sem_free_slots, sem_data_items;
	xsem_init(&sem_free_slots, 0, PC_buffer_len, QUI);
	xsem_init(&sem_data_items, 0, 0, QUI);
	// dichiarare la mux
	pthread_mutex_t mu = PTHREAD_MUTEX_INITIALIZER;
	// dichiarare il buffer
	char *buffer[PC_buffer_len];
	dati_scrittore a[3];

	//chiamiamo tre thread scrittori
	for(int i = 0; i < 3; i++){
		a[i].index = &cindex;
		a[i].sem_free_slots = &sem_free_slots;
		a[i].sem_data_items = &sem_data_items;
		a[i].buffer = buffer;
		a[i].mutex = &mu;
		xpthread_create(&t[i], NULL, *scrittore, &a[i], QUI);
	}

	//diamogli in pasto le stringhe
	for(int i = string_num; i < string_num + 3; i++){
		stringhe[i] = NULL;
	}
	printf("Il numero di stringhe è %d", string_num);
	for(int i = 0; i< string_num+3; i++){
		printf("Sto passando la stringa %s agli scrittori\n", stringhe[i]);
		xsem_wait(&sem_free_slots, QUI);
		xpthread_mutex_lock(&mu, QUI);
		buffer[pindex % PC_buffer_len] = stringhe[i];
		pindex++;
		xpthread_mutex_unlock(&mu, QUI);
		xsem_post(&sem_data_items, QUI);
	}

	char s[11];
	printf("Cerca una stringa: \n");
	scanf(" %10[^\n]", s);
	while(strcasecmp(s, "fine") != 0){
		int res = conta(s);
		printf("Trovata %d volte\n", res);
		scanf(" %10[^\n]", s);
	}

	hdestroy();
	
	printf("Stringhe uniche: %d", stringhe_uniche);
	for(int i = 0; i<string_num; i++){
		free(stringhe[i]);
	}
	free(stringhe);
	return 0;
}

