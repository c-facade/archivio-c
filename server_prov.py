#!/usr/bin/env python3

'''
Manda delle stringhe a caso sulla fifo caposc
e poi sulla fifo capolet
'''

import sys, os, struct

'''
Funzioni che utilizzo
    os.mkfifo(path)         # crea una fifo
    os.unlink(path)         # cancella un file
    os.open(path, flags)    # open
    os.read(fd, n)          # read dal file descriptor n bytes
    struct.pack(format, val)    # trasforma int -> sequenza byte
    struct.unpack(format, bs)   # trasforma sequanza byte in int
'''

# Variabili globali con i nomi delle pipe da usare

Caposcrittore = "caposc"
Capolettore = "capolet"

def main(file_scritture, file_letture):
   
    os.mkfifo(Caposcrittore)

    # apriamo le pipe
    cs = os.open(Caposcrittore, os.O_WRONLY)

    # apriamo il file di testo contenente 
    # le stringhe da aggiungere

    print("Inizio lettura")

    with open(file_scritture, "r") as fs:
        for linea in fs:
            # converto la linea in una sequenza di byte
            # e le scrivo nella pipe
            
            # prima invio la lunghezza della stringa
            bs = struct.pack("<i", len(linea))
            os.write(cs, bs)
            l = str.encode(linea);
            for char in l:
                # print("Char =", char, "type = ", type(char))
                bs = struct.pack("b", char)
                os.write(cs, bs)

    print("file inviato a caposcrittore")
    # probabilmente questo non è necessario
    bs = struct.pack("<i", 0);
    os.close(cs)
    os.unlink(Caposcrittore)

    # ora diamogli da leggere
   
    os.mkfifo(Capolettore)
    cl = os.open(Capolettore, os.O_WRONLY)

    with open(file_letture, "r") as fl:
        for linea in fl:
            # converto la linea in una sequenza di byte
            # e la scrivo nella pipe

             # prima invio la lunghezza della stringa
            bs = struct.pack("<i", len(linea))
            os.write(cl, bs)
            l = str.encode(linea);
            for char in l:
                bs = struct.pack("b", char)
                os.write(cl, bs)

    print("file inviato a capolettore");
    os.close(cl);
    os.unlink(Capolettore)

# lancio del main

if len(sys.argv)!=3:
    print("Uso:\n\t %s file_scritture file_letture" % sys.argv[0])
else:
    main(sys.argv[1], sys.argv[2])

