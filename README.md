# archivio-c
Progetto finale di laboratorio 2

L'obbiettivo del progetto è di realizzare un server concorrente che invia linee di testo ad un programma Archivio, il quale gestisce la memorizzazione di stringhe in una tabella hash.

Il codice è suddiviso nei files `hash.c`, `lettori.c`, `scrittori.c`, `archivio.c`, `server.py`, `client1`, `client2`. Il codice C utilizza le funzioni per la gestione degli errori che abbiamo visto in classe, nel file `xerrori.c`.

## Hash.c
L'accesso alla tabella hash è gestito con uno schema lettori-scrittori che utilizza le condition variables. Permette una scrittura alla volta, ma letture contemporanee tra loro.
Le funzioni aggiungi e conta sono utilizzate per la scrittura e lettura in tabella, rispettivamente.
La struct `sync` contiene i dati per la sincronizzazione, che sono il numero di lettori, una variabile booleana che indica se è in corso una scrittura, la condition variable che permette di regolare gli accessi, e un mutex.

## Lettori.c
Il thread capolettore apre il file di logging, inizializza le variabili di sincronizzazione, fa partire i thread lettori, si collega con il server attraverso la pipe `capolet`, da cui riceve le stringhe. Le manda ai server lettori attraverso un buffer produttore-consumatori implementato con i semafori.
La struct dati_lettore contiene il necessario per leggere dal buffer PC, il file descriptor del file `lettori.log`, un mutex per sincronizzare la scrittura nel file, e la struct `sync` da passare come parametro alla funzione conta.
Il thread lettore legge la stringa da buffer, invoca la funzione conta, e scrive sul file `lettori.log` la stringa e il suo numero di occorrenze.

## Scrittori.c
Il funzionamento del caposcrittore è analogo a quello del capolettore. Il caposcrittore riceve come argomento anche il puntatore al numero di stringhe uniche. I threads scrittori invocano la funzione `aggiungi` con il numero di stringhe uniche come argomento.

# Archivio.c
Questo file contiene il main e il codice per la gestione di segnali.
Il main invoca i threads gestore di segnali, poi il caposcrittore e il capolettore, poi entra in un loop infinito.
Termina solo se riceve il segnale SIGTERM.

# Server.py
Il server in python crea le pipe `caposc` e `capolet`, fa partire archivio.c come sottoprocesso, apre le pipe.
Accetta connessioni, assegnando ad ogni connessione un thread, gestito con ThreadPoolExecutor.
Quando il client si collega, invia il carattere 'A' per comunicare che si tratta di una connessione di tipo A, e il carattere 'B' per una connessione di tipo B.
Nel caso di una connessione di tipo A, il server riceve una singola linea di un file e la invia su `caposc`.
Altrimenti entra in un loop e riceve stringhe, inviandole su `capolet`, finchè non ne riceve una di lunghezza zero.
In entrambi i casi, scrive sulla pipe una struct che contiene la lunghezza della stringa encoded, che è uno short int, e la stringa in sè. La stringa e la sua lunghezza sono scritte atomicamente.
Quando il server riceve il segnale SIGINT, tramite l'eccezione KeyboardInterrupt, smette di ricevere connessioni, chiude le named pipes, manda SIGTERM al sottoprocesso archivio.c, chiude il server ed elimina le pipe.

# client1
File eseguibile scritto in python. Legge linee da un file e le invia al server tramite una connessione di tipo A.

# client2
File eseguibile scritto in python. Riceve in input uno o più file. Per ogni file, crea un thread che lo legge e lo invia, una riga alla volta, tramite una connessione di tipo B.
I threads sono gestiti con ThreadPoolExecutor.
