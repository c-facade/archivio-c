#!/usr/bin/env python3

'''
server.py
File python eseguibile che ha i seguenti argomenti
necessario: numero massimo di thread
opzionali:
    -r il numero di lettori
    -w numero di scrittori
        (default 3)
    -v per chiamare con valgrind
Usa subprocess.Popen per aprire archivio
gestisce l'eccezione keyboard interrupt
'''

import argparse
import logging
import socket
import os

HOST = "127.0.0.1"
PORT = 501433

def server(numT, r, w, v):
    logging.basicConfig(filename=os.path.basename+ "server.log", level=logging.DEBUG, datefmt="%d/%m/%y %H:%M:%S", format="%(asctime)s - %(levelname)s - %(message)s")

    # ti devi guardare socket/contaprimi.py per vedere come si
    # fa la cosa di avere diversi clients in un thread dedicato

    # creazione del server socket
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.bind( (HOST, PORT) )
        s.listen()
        while True:
            # Ad ogni client che si connette, il server deve assegnare un thread dedicato.
            # Bisogna usare ThreadPoolExecutor
            print(f"In attesa di un client sulla porta {PORT}");
            # mi metto in attesa di una connessione
            conn, addr = s.accept()
            #lavoro con la connessione appena ricevuta
            with conn:
                print(f"Contattato da {addr}")
                while True:
                    # vediamo cosa fare
                    data = conn.recv(64)

                    if not data:
                        break;
                print("Connessione terminata")

if __name__ == "__main__":
    parser = argparse.ArgumentParser(prog="server.py", description="server che chiama archivio.c")
    parser.add_argument("numT", help="il numero massimo di thread");
    parser.add_argument("-r", help="il numero di lettori(escluso capolettore)")
    parser.add_argument("-w", help="il numero di scrittori(escluso caposcrittore)")
    parser.add_argument("-v", action="store_true", help="Testa con valgrind")

    args = parser.parse_args()
    
    server(args.numT, args.r, args.w, args.v);
