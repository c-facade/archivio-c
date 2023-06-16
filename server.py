#!/usr/bin/env python3

'''
server.py
File python eseguibile che ha i seguenti argomenti:
    max_threads (numero massimo di thread allo stesso tempo)
    -r (numero di lettori) (default 3)
    -w (numero di scrittori) (default 3)
    -v (per chiamare archivio.c con valgrind)

Questo server riceve delle stringhe da client1 e client2
via socket, attraverso connessioni di tipo A e B
Le stringhe ricevute sono inviate via pipe al sottoprocesso
archivio.c.
'''

import os, struct, socket, threading, concurrent.futures
import subprocess, signal, argparse, logging, socket

HOST = "127.0.0.1"
PORT = 51433
Caposcrittore = "caposc"
Capolettore = "capolet"

Descrizione = "Questo server fa partire archivio.c come sottoprocesso, riceve stringhe da connessioni di tipo A e B, e le invia ad archivio.c attraverso le pipe capolet e caposc"

def main(max_threads):
   
    if not os.path.exists(Caposcrittore):
        os.mkfifo(Caposcrittore)
    if not os.path.exists(Capolettore):
        os.mkfifo(Capolettore)

    cs = os.open(Caposcrittore, os.O_WRONLY)
    cl = os.open(Capolettore, os.O_WRONLY)

    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        try:
            s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
            s.bind((HOST, PORT))
            s.listen()
            while(True):
                with concurrent.futures.ThreadPoolExecutor(max_workers=max_threads) as executor:
                    print("In attesa di un client...")
                    conn, addr = s.accept()
                    executor.submit(gestisci_connessione, conn, addr, cs, cl)
        except KeyboardInterrupt:
            pass
        print("Terminazione")
        os.close(cs)
        os.close(cl)
        s.shutdown(socket.SHUT_RDWR)

def gestisci_connessione(conn, addr, cs, cl):
    with conn:
        print(f"{threading.current_thread().name} contattato da {addr}")
        tipo = (struct.unpack("c", recv_all(conn, 1))[0]).decode()
        print(f"{threading.current_thread().name} tipo: ", tipo)
        if(tipo == 'A'):
            l = recv_all(conn, 4)
            assert len(l) == 4
            lunghezza = struct.unpack("!i", l)[0]
            print("lunghezza =", lunghezza)
            lun = struct.pack("<i", lunghezza)
            e1 = os.write(cl, lun)
            data = recv_all(conn, lunghezza)
            assert len(data) == lunghezza
            stringa = data.decode()
            print(f"{threading.current_thread().name} Ricevuto:", stringa)
            e2 = os.write(cl, data)
            print(e1, e2)
        else:
            while True:
                l = recv_all(conn, 4)
                assert len(l) == 4
                lunghezza = struct.unpack("!i", l)[0]
                print("lunghezza = ", lunghezza)
                lun = struct.pack("<i", lunghezza)
                e1 = os.write(cs, lun)
                if lunghezza == 0:
                    print(f"{threading.current_thread().name} Terminare connessione di tipo B")
                    break
                data = recv_all(conn, lunghezza)
                assert len(data) == lunghezza
                stringa = data.decode()
                print(f"{threading.current_thread().name} Ricevuto:", stringa)
                e2 = os.write(cs, data)
                print(e1, e2)


def recv_all(conn, n):
    chunks = b''
    bytes_recd = 0
    while bytes_recd < n:
        chunk = conn.recv(min(n- bytes_recd, 1024))
        #print("chunk = ", chunk)
        if len(chunk) == 0:
            raise RuntimeError("socket connection broken")
        chunks += chunk
        bytes_recd = bytes_recd + len(chunk)
    # print("chunks = ", chunks)
    return chunks

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=Descrizione)
    parser.add_argument("max_threads", help="numero massimo di thread che gestiscono connessioni.", type=int)
    parser.add_argument("-r", help="numero di thread lettori", type=int, default=3)
    parser.add_argument("-w", help="numero di thread scrittori", type=int, default=3)
    parser.add_argument("-v", action="store_true", help="esegui con valgrind")
    args = parser.parse_args()
    assert args.max_threads > 0
    assert args.r > 0
    assert args.w > 0
    print(args)
    main(args.max_threads)
