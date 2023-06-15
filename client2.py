import struct, socket, concurrent.futures, threading

HOST = "127.0.0.1"
PORT = 51433

def main(file_list):
    with concurrent.futures.ThreadPoolExecutor(max_workers=5) as executor:
        for file in file_list:
            executor.submit(invia_file, file)

def invia_file(file):
    with open(file, "r") as f:
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
            s.connect((HOST, PORT))
            print(f"{threading.current_thread().name} Connesso a", s.getpeername())
            # pronto per inviare la richiesta
            b = str.encode("B")
            s.sendall(struct.pack("c", b))
            for linea in f:
                line = linea.encode()
                lunghezza = len(line);
                s.sendall(struct.pack("!i", lunghezza))
                for carattere in line:
                    s.sendall(struct.pack("b", carattere))
                print(f"{threading.current_thread().name} Inviato con successo.")
            print(f"{threading.current_thread().name} -- Inviato intero file.\n Invio il segnale di terminazione.")
            lunghezza = 0
            s.sendall(struct.pack("!i", lunghezza))
            s.shutdown(socket.SHUT_RDWR)

main(["file1", "file1"])
