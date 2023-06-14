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

si collega a client di tipo A e B,
per ogni connessione usa un thread.
'''

import sys, os, struct
import subprocess, signal
import argparse, logging, socket
import threading
import concurrent.futures

# Variabili globali

Caposcrittore = "caposc"
Capolettore = "capolet"
HOST = "127.0.0.1"
PORT = 51433

# per ora gli facciamo solo stampare quello che arriva
# da una connessione di tipo A

def main(host = HOST, port = PORT):
    # creiamo il server socket
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        try:
            s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
            s.bind((host, port))
            s.listen()
            with concurrent.futures.ThreadPoolExecutor(max_workers = 5) as executor:
                while True:
                    print("In attesa di un client...")
                    conn, addr = s.accept()
                    executor.submit(gestisci_connessione, conn, addr)
        except KeyboardInterrupt:
            pass
        print("Terminazione...")
        s.shutdown(socket.SHUT_RDWR)

def gestisci_connessione(conn, addr):
    with conn:
        print(f"{threading.current_thread().name} contattato da {addr}")
        tipo = (struct.unpack("c", recv_all(conn, 1))[0]).decode()
        if(tipo == 'A'):
            print("Connessione di tipo A.")
            # mi faccio inviare la lunghezza della stringa
            data = recv_all(conn, 4)
            assert len(data) == 4
            lunghezza = struct.unpack("!i", data)[0]
            assert lunghezza < 1024
            data = recv_all(conn, lunghezza)
            stringa = data.decode()
            print("Ricevuto: ", stringa)
        else:
            print("Connessione di tipo B.")
            while True:
        		# mi faccio di nuovo inviare la lunghezza della stringa
                data = recv_all(conn, 4)
                assert len(data) == 4
                lunghezza = struct.unpack("!i", data)[0]
                print("La lunghezza della stringa è", lunghezza)
                assert lunghezza < 2048
                if lunghezza == 0:
                    print("Terminare connessione di tipo B")
                    break;
                data = recv_all(conn, lunghezza)
                stringa = data.decode()
                print("Ricevuto: ", stringa)


def recv_all(conn, n):
    chunks = b''
    bytes_recd = 0
    while bytes_recd < n:
        chunk = conn.recv(min(n- bytes_recd, 1024))
        if len(chunk) == 0:
            raise RuntimeError("socket connection broken")
        chunks += chunk
        bytes_recd = bytes_recd + len(chunk)
    return chunks


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument("max_threads", help="numero massimo di threads", type=int)
    parser.add_argument("-r", help="numero di thread lettori", type = int)
    parser.add_argument("-w", help = "numero di thread scrittori", type = int)
    parser.add_argument("-v")

