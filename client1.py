# client1.py

# qui inviamo la lunghezza della sequenza di byte
# e poi i byte
# semplice

import socket, struct
import sys

HOST = "127.0.0.1"
PORT = 51433

def main(file, host = HOST, port = PORT):
    with open(file, "r") as f:
        for linea in f:
            line = str.encode(linea)
            lunghezza = len(line)
            with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
                s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
                # la prossima chiamata è blocking
                s.connect((host, port))
                print("Connesso a ", s.getpeername())
                # pronto per inviare la richiesta
                s.sendall(struct.pack("!i", lunghezza))
                for carattere in line:
                    s.sendall(struct.pack("b", carattere))
                s.shutdown(socket.SHUT_RDWR)


if len(sys.argv) == 2:
    main(sys.argv[1])


