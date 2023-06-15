#!/usr/bin/env python3


# Il server si metterà in attesa in un ciclo while e aspetterà che gli
# si invii:
# lunghezza sequenza - sequenza di byte
# fino a che non arriva una di lunghezza 0.

import struct, socket, argparse, concurrent.futures
import sys

HOST = "127.0.0.1"
PORT = 51433

def gestisci_connessione(file, host = HOST, port = PORT):
    print("gestendo connessione")
    # inizializzazione del socket client
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        s.connect((host, port))
        print("Connesso a", s.getpeername())
        b = str.encode("B")
        s.sendall(struct.pack("c", b))
        print("Inviata la b")
        with open(file, "r") as f:
            for linea in f:
                print("Inviando linea: ", linea)
                line = str.encode(linea)
                print("Line: ", line, "type: ", type(line))
                lunghezza = len(line)
                print("Lunghezza:", lunghezza)
                s.sendall(struct.pack("!i", lunghezza))
                print("Inviata lunghezza con successo.")
                for carattere in line:
                    print("carattere =", carattere, "type=", type(carattere), "len =", len(carattere))
                    s.sendall(struct.pack("b", carattere))
                print("Linea inviata.")
            print("Trasferito file.")
            empty = struct.pack("b", str.encode(""))
            s.sendall(empty)
        s.shutdown(socket.SHUT_RDWR)

def main(file_list):
    print("main")
    assert len(file_list) > 0
    print(file_list)
    with concurrent.futures.ProcessPoolExecutor(max_workers=len(file_list)) as executor:
        for file in file_list:
            executor.submit(gestisci_connessione, file)

if len(sys.argv) > 1:
    print("hello")
    main(sys.argv[1:])
else:
    print("uso: client2 file1 file2...")
