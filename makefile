# definizione del compilatore e dei flag di compilazione
# che vengono usate dalle regole implicite
CC=gcc
CFLAGS=-std=c11 -Wall -g -O -pthread
LDLIBS=-lm -lrt -pthread

all: archivio.out

archivio.out:
	gcc -Wall -g archivio.c xerrori.c scrittori.c lettori.c hash.c -o archivio.out
