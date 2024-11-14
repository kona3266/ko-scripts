import threading
import socket
import logging
import time

def client(host, port, name):
    sockobj = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sockobj.connect((host, port))
    logging.info('{0} connected...'.format(name))
    sockobj.send(b'2305843009213693951@')
    buf = sockobj.recv(8 * 1024)
    logging.info('{0} received {1}'.format(name, buf))
    sockobj.close()
    logging.info('{0} disconnecting'.format(name))

if __name__ == "__main__":
    host = "localhost"
    port = 8070
    connections = []
    logging.basicConfig(
        level=logging.DEBUG,
        format='%(levelname)s:%(asctime)s:%(message)s')
    j = 1
    maxN = 100
    prev_time = None
    while j <= maxN:
        t1 = time.time()
        connections = []
        for i in range(j):
            name = 'conn{0}'.format(i)
            tconn = threading.Thread(target=client,
                                    args=(host, port, name))
            tconn.start()
            connections.append(tconn)

        for conn in connections:
            conn.join()
        j = j + 1
        work_time = time.time() - t1
        print('%s thread Elapsed:', j, work_time)
        if prev_time is not None:
            if work_time >= 1.5* prev_time:
                print('server thread work is full:', j)
                break
        prev_time = work_time
