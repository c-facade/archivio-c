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
import time

HOST = "127.0.0.1"
PORT = 51433
Caposcrittore = "caposc"
Capolettore = "capolet"
Max_sequence_length = 2048
# ricorda: in linux, PIPE_BUF è 4096 bytes.

Descrizione = "Questo server fa partire archivio.c come sottoprocesso, riceve stringhe da connessioni di tipo A e B, e le invia ad archivio.c attraverso le pipe capolet e caposc"

def main(max_threads, readers, writers, valgrind):
    
    logging.basicConfig(filename="server.log", level=logging.INFO, datefmt="%d/%m/%y %H:%M:%S", format="%(asctime)s - %(levelname)s - %(message)s")

    # se non esistono già crea le pipe
    if not os.path.exists(Caposcrittore):
        os.mkfifo(Caposcrittore)
    if not os.path.exists(Capolettore):
        os.mkfifo(Capolettore)
   
    if valgrind:
        p = subprocess.Popen(["valgrind", "--leak-check=full", "--show-leak-kinds=all", "--log-file=valgrind-%p.log", "./archivio", str(readers), str(writers)])
    else:
        p = subprocess.Popen(["./archivio", str(readers), str(writers)])
        pass

    logging.debug("Ho fatto partire archivio.")

    # le pipe restano aperte finchè non si dà segnale
    # di terminazione
    cs = os.open(Caposcrittore, os.O_WRONLY)
    cl = os.open(Capolettore, os.O_WRONLY)

    # ogni connessione ha il suo thread dedicato
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        try:
            s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
            s.bind((HOST, PORT))
            s.listen()
            while(True):
                with concurrent.futures.ThreadPoolExecutor(max_workers=max_threads) as executor:
                    #logging.debug("In attesa di un client...")
                    conn, addr = s.accept()
                    executor.submit(gestisci_connessione, conn, addr, cs, cl)
        except KeyboardInterrupt:
            pass
        logging.debug("server.py -- In terminazione")
        # chiude le pipe
        # causando a caposcrittore e capolettore di terminare
        os.close(cs)
        os.close(cl)
        # invia sigterm al main di archivio.c
        p.send_signal(signal.SIGTERM)
        # attende la terminazione
        time.sleep(1)
        logging.debug("Exit code di Archivio o None se non è terminato: %s", p.poll())
        # elimina le pipe
        os.unlink(Caposcrittore)
        os.unlink(Capolettore)
        # chiude il server
        s.shutdown(socket.SHUT_RDWR)
        logging.debug("Server terminato.")

def gestisci_connessione(conn, addr, cs, cl):
    # questa funzione può gestire connessioni di tipo A e B
    with conn:
        logging.debug("%s contattato da %s", threading.current_thread().name, addr)
        # il client invia una 'A' se la connessione è di tipo A
        # altrimenti una B, subito dopo essersi connesso
        tipo = (struct.unpack("c", recv_all(conn, 1))[0]).decode()
        #logging.debug("Connessione di tipo %s", tipo)
        bytes_inviati = 0
        if(tipo == 'A'):
            # per ogni riga si invia la lunghezza
            l = recv_all(conn, 2)
            assert len(l) == 2
            lunghezza = struct.unpack("<h", l)[0]
            assert lunghezza < Max_sequence_length
            data = recv_all(conn, lunghezza)
            assert len(data) == lunghezza
            stringa = data.decode()
            # scrivo la stringa sulla pipe
            # faccio una struct in modo da mandare nella pipe
            # in modo atomico la lunghezza e la stringa
            packet = struct.pack(f"<h{lunghezza}s", lunghezza, data)
            logging.debug("mandando: %s su capolet", packet);
            e = os.write(cl, packet)
            bytes_inviati = bytes_inviati + int(e)
        else:
            while True:
                l = recv_all(conn, 2)
                assert len(l) == 2
                lunghezza = struct.unpack("<h", l)[0]
                if lunghezza == 0:
                    break
                assert lunghezza < Max_sequence_length
                data = recv_all(conn, lunghezza)
                assert len(data) == lunghezza
                packet = struct.pack(f"<h{lunghezza}s", lunghezza, data)
                logging.debug("Mandando %s su caposc", packet)
                e = os.write(cs, packet)
                bytes_inviati = bytes_inviati + int(e)
        logging.info("Connessione di tipo %s, inviati %s bytes", tipo, bytes_inviati)


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
    parser = argparse.ArgumentParser(description=Descrizione)
    parser.add_argument("max_threads", help="numero massimo di thread che gestiscono connessioni.", type=int)
    parser.add_argument("-r", help="numero di thread lettori", type=int, default=3)
    parser.add_argument("-w", help="numero di thread scrittori", type=int, default=3)
    parser.add_argument("-v", action="store_true", help="esegui con valgrind")
    args = parser.parse_args()
    assert args.max_threads > 0
    assert args.r > 0
    assert args.w > 0
    main(args.max_threads, args.r, args.w, args.v)
