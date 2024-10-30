from socket import *
import protobuf.zmq_socket.target_pb2 as target_pb2
from protobuf.utils import decode_varint
import threading

HOST = "localhost"

class SimpleHandler(object):
    def __init__(self, request, client_address, server):
        self.request = request
        self.client_address = client_address
        self.server = server
        self.setup()
        try:
            self.handle()
        finally:
            self.finish()

    def handle(self):
        # receive size in head first
        message = b''
        while True:
            try:
                message += self.request.recv(1)
                size = decode_varint(message)
                break
            except IndexError:
                pass
        # receive the body data
        data = self.request.recv(size)
        recv_rec = target_pb2.Rect()
        recv_rec.ParseFromString(data)
        print(recv_rec.x1, recv_rec.y1)
        print(recv_rec.x2, recv_rec.y2)
        self.wfile.write("new message")

    def setup(self):
        self.connection = self.request
        self.rfile = self.connection.makefile('rb', -1)
        self.wfile = self.connection.makefile('wb', 0)

    def finish(self):
        if not self.wfile.closed:
            try:
                self.wfile.flush()
            except socket.error:
                # A final socket error may have occurred here, such as
                # the local error ECONNABORTED.
                pass
        self.wfile.close()
        self.rfile.close()


class ThreadingMixIn(object):
    """Mix-in class to handle each request in a new thread."""

    # Decides how threads will act upon termination of the
    # main process
    daemon_threads = False

    def process_request_thread(self, request, client_address):
        self.finish_request(request, client_address)
        self.shutdown_request(request)


    def process_request(self, request, client_address):
        t = threading.Thread(target = self.process_request_thread,
                             args = (request, client_address))
        t.daemon = self.daemon_threads
        t.start()



class BaseServer(object):
    def __init__(self, handler_class,  port=5555, max_conn=1000):
        self.handler_class = handler_class
        self.port = port
        self.addr = (HOST, self.port)
        #listen的参数代表，在连接被拒绝或者转发之前，传入连接的最大数
        self.max_conn = max_conn
        self.available_worker = []
        self.tcp_sock = None
        self.is_full = False

    def listen(self):
        tcpSerSock = socket(AF_INET, SOCK_STREAM)
        tcpSerSock.bind(self.addr)
        tcpSerSock.listen(self.max_conn)
        self.tcp_sock = tcpSerSock

    def shutdown_request(self, request):
        """Called to shutdown and close an individual request."""
        try:
            #explicitly shutdown.  socket.close() merely releases
            #the socket and waits for GC to perform the actual close.
            request.shutdown(SHUT_WR)
        except socket.error:
            pass #some platforms may raise ENOTCONN here
        self.close_request(request)

    def close_request(self, request):
        """Called to clean up an individual request."""
        request.close()

    def _handle_request_noblock(self):
        cli_request_sock, client_address = self.get_request()
        print("... connected from:", client_address)
        self.process_request(cli_request_sock, client_address)

    def process_request(self, request, client_address):
        self.finish_request(request, client_address)
        self.shutdown_request(request)

    def finish_request(self, request, client_address):
        self.handler_class(request, client_address, self)

    def get_request(self):
        return self.tcp_sock.accept()

    def serve(self):
        try:
            while True:
                print("waiting for connnection ...")
                self._handle_request_noblock()
        finally:
            print("server shut down")

class ThreadingBaseServer(ThreadingMixIn, BaseServer):pass

if __name__ == "__main__":
    s = ThreadingBaseServer(SimpleHandler)
    s.listen()
    s.serve()
