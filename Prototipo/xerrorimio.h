#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <search.h>
#include <strings.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>

void termina(const char *s);
void xtermina(const char *s, int linea, char *file);

int xpthread_create(pthread_t *thread, const pthread_attr_t *attr, void *(*start_routine) (void *), void *arg, int linea, char *file);

int xpthread_join(pthread_t thread, void **retval, int linea, char * file);


