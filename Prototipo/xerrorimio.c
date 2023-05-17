#include "xerrorimio.h"

void termina(const char *messaggio){
	if(errno == 0){
		fprintf(stderr, "== %d == %s\n", getpid(), messaggio);
	}
	else{
		fprintf(stderr, "== %d == %s: %s\n", getpid(), messaggio, strerror(errno));
	}
	exit(1);
}

void xtermina(const char *messaggio, int linea, char *file){
	if(errno == 0){
		printf(stderr, "== %d == %s\n", getpid(), messaggio);
	}
	else{
		fprintf(stderr, "== %d == %s: %s\n", getpid(), messaggio(), strerror(errno));
	}
	fprintf(stderr, "== %d == Linea: %d, File: %s\n", getpid(), linea, file);

	exit(1);
}

// funzioni per thread

#define Buflen 100
void xperror(int en, char *msg) {
	char buf[Buflen];

	char *errmsg = strerror_r(en, buf, Buflen);
	if(msg!= NULL)
		fprintf(stderr, "%s: %s\n", msg, errmsg);
	else
		fprintf(stderr, "%s\n", errmsg);
}

//threads: creazione e join

int xpthread_create(pthread_t *thread, const pthread_attr_t *attr,
                          void *(*start_routine) (void *), void *arg, int linea, char *file) {
  int e = pthread_create(thread, attr, start_routine, arg);
  if (e!=0) {
    xperror(e, "Errore pthread_create");
    fprintf(stderr,"== %d == Linea: %d, File: %s\n",getpid(),linea,file);     pthread_exit(NULL);
  }
  return e;
}

int xpthread_join(pthread_t thread, void **retval, int linea, char *file) {
  int e = pthread_join(thread, retval);
  if (e!=0) {
    xperror(e, "Errore pthread_join");
    fprintf(stderr,"== %d == Linea: %d, File: %s\n",getpid(),linea,file);
    pthread_exit(NULL);
  }
  return e;
}

