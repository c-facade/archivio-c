# definizione del compilatore e dei flag di compilazione
# che vengono usate dalle regole implicite
CC=gcc
CFLAGS=-std=c11 -Wall -g -O -pthread
LDLIBS=-lm -lrt -pthread

all: archivio.out

archivio.out: archivio.c xerrori.c scrittori.c lettori.c hash.c
	$(CC) $(CFLAGS) $^ -o $@ $(LDLIBS)

archivio.o: archivio.c lettoriescrittori.h
	$(CC) $(LDFLAGS) -c $<

lettori.o: lettori.c hash.h lettoriescrittori.h
	$(CC) $(CFLAGS) -c $<

scrittori.o: scrittori.c hash.h lettoriescrittori.h
	$(CC) $(CFLAGS) -c $<

hash.o: hash.c hash.h xerrori.h
	$(CC) $(CFLAGS) -c $<

xerrori.o: xerrori.c xerrori.h
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f *.o *.out

# gcc -Wall -g archivio.c xerrori.c scrittori.c lettori.c hash.c -o archivio.out
