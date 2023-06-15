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

import os, struct
import subprocess, signal
import argparse, logging, socket
import threading
import concurrent.futures
import time

# Variabili globali

Caposcrittore = "caposc"
Capolettore = "capolet"
HOST = "127.0.0.1"
PORT = 51433

Description = "Questo server si connette a client di tipo A e B, riceve delle stringhe, e le manda ad archivio.c attraverso le pipe capolettore e caposcrittore."

def main(numT, r, w, v, host = HOST, port = PORT):
    
    logging.basicConfig(filename="server.log", level=logging.DEBUG, datefmt="%d/%m/%y %H:%M:%S", format="%(asctime)s - %(levelname)s - %(message)s")

    if not os.path.exists(Caposcrittore):
        os.mkfifo(Caposcrittore)
    if not os.path.exists(Capolettore):
        os.mkfifo(Capolettore)

    logging.debug("Create le pipe")
    
    '''
    if v:
        command = ["valgrind", "--leak-check=full", "--show-leak-kinds=all", "--log-file=valgrind.%p.log", "./archivio.out", str(r), str(w)]
    else:
        command = ["./archivio.out", str(r), str(w)]
    p = subprocess.Popen(command)
    '''

    logging.debug("Ho fatto partire archivio")

    cs = os.open(Caposcrittore, os.O_WRONLY)
    cl = os.open(Capolettore, os.O_WRONLY)

    logging.debug("Ho aperto le pipe")

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
                    executor.submit(gestisci_connessione, conn, addr, cs, cl)
        except KeyboardInterrupt:
            pass
        print("Terminazione...")
        #p.send_signal(signal.SIGTERM)
        s.shutdown(socket.SHUT_RDWR)
        os.close(cs)
        os.close(cl)
        os.unlink(Caposcrittore)
        os.unlink(Capolettore)
        time.sleep(1)
        #print("Exit code di main o None se non è terminato:", p.poll())

def gestisci_connessione(conn, addr, cs, cl):
    with conn:
        print(f"{threading.current_thread().name} contattato da {addr}")
        tipo = (struct.unpack("c", recv_all(conn, 1))[0]).decode()
        if(tipo == 'A'):
            logging.debug("Connessione di tipo A.")
            # mi faccio inviare la lunghezza della stringa
            data = recv_all(conn, 4)
            assert len(data) == 4
            lunghezza = struct.unpack("!i", data)[0]
            assert lunghezza < 2048
            os.write(cl, data)
            data = recv_all(conn, lunghezza)
            stringa = data.decode()
            logging.debug("Ricevuto: "+stringa)
            os.write(cl, data)
        else:
            print(threading.current_thread().name, "-- Connessione di tipo B.")
            while True:
                print(threading.current_thread().name, "-- ricevendo da client 2")
                # mi faccio di nuovo inviare la lunghezza della stringa
                data = recv_all(conn, 4)
                print(threading.current_thread().name, "-- ricevuta lunghezza.")
                assert len(data) == 4
                print(threading.current_thread().name, "-- len data = 4")
                lunghezza = struct.unpack("!i", data)[0]
                print(threading.current_thread().name, "-- La lunghezza della stringa è "+lunghezza)
                # assert lunghezza < 2048
                if lunghezza == 0:
                    logging.debug("Terminare connessione di tipo B")
                    break
                # os.write(cs, data)
                data = recv_all(conn, lunghezza)
                stringa = data.decode()
                print(threading.current_thread().name, "-- Ricevuto: "+stringa)
                # os.write(cs, data)


def recv_all(conn, n):
    chunks = b''
    bytes_recd = 0
    while bytes_recd < n:
        chunk = conn.recv(min(n- bytes_recd, 1024))
        print("chunk = ", chunk)
        if len(chunk) == 0:
            raise RuntimeError("socket connection broken")
        chunks += chunk
        bytes_recd = bytes_recd + len(chunk)
    print("chunks = ", chunks)
    return chunks


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description = Description)
    parser.add_argument("max_threads", help="numero massimo di threads", type=int)
    parser.add_argument("-r", help="numero di thread lettori", type = int, default=3)
    parser.add_argument("-w", help = "numero di thread scrittori", type = int, default=3)
    parser.add_argument("-v", action="store_true",  help="esegui con valgrind")
    args = parser.parse_args()
    assert args.max_threads > 0
    main(args.max_threads, args.r, args.w, args.v)
