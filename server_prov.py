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

    with open(file_scritture, "r") as fs:
        for linea in fs:
            # converto la linea in una sequenza di byte
            # e le scrivo nella pipe
            
            # prima invio la lunghezza della stringa
            bs = struct.pack("<i", len(linea))
            os.write(fs, bs)
            for char in linea:
                bs = struct.pack("c", char)
                os.write(cs, bs)

    # ora diamogli da leggere

    cl = os.open(Capolettore, os.O_WRONLY)

    with open(file_letture, "r") as fl:
        for linea in fl:
            # converto la linea in una sequenza di byte
            # e la scrivo nella pipe

            bs= struct.pack("<i", len(linea))

            os.write(fl, bs)
            for char in linea:
                bs = struct.pack("c", char)
                os.write(cl, bs)
    os.unlink(Caposcrittore)

# lancio del main

if len(sys.argv)!=3:
    print("Uso:\n\t %s file_scritture file_letture" % sys.argv[0])
else:
    main(sys.argv[1], sys.argv[2])

