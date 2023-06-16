#!/usr/bin/env python3

import os, struct, socket, threading, concurrent.futures

HOST = "127.0.0.1"
PORT = 51433

def main(host = HOST, port=PORT):
   
    if not os.path.exists("caposc"):
        os.mkfifo("caposc")
    if not os.path.exists("capolet"):
        os.mkfifo("capolet")

    cs = os.open("caposc", os.O_WRONLY)
    cl = os.open("capolet", os.O_WRONLY)

    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        try:
            s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
            s.bind((host, port))
            s.listen()
            while(True):
                with concurrent.futures.ThreadPoolExecutor(max_workers=5) as executor:
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

main()
