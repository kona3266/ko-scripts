import socket
import threading
import protobuf.zmq_socket.target_pb2 as target_pb2
from protobuf.utils import encode_varint, decode_varint

rect = target_pb2.Rect()
rect.x1 = 0
rect.y1 = 2
rect.x2 = 2
rect.y2 = 0
HOST = "localhost"


class Client():
    def __init__(self, port=5555):
        self.conn = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.conn.connect((HOST, port))
    
    def send(self, msg):
        data = msg.SerializeToString()
        size = encode_varint(len(data))
        self.conn.sendall(size + data)
    
    def recv_message(self, msg_type):
        # Receive the size of the message data
        data = b''
        while True:
            try:
                data += self.conn.recv(1)
                size = decode_varint(data)
                break
            except IndexError:
                pass
        # Receive the message data
        data = self.conn.recv(size)
        # Decode the message
        msg = msg_type()
        msg.ParseFromString(data)
        return msg
    
def wait_reply(msg):
    c = Client()
    c.send(msg)
    m = c.recv_message(target_pb2.Rect)
    print(m.x1, m.y1, m.x2, m.y2)


for i in range(4):
    rect = target_pb2.Rect()
    rect.x1 = i
    rect.y1 = i+1
    rect.x2 = i+2
    rect.y2 = i+3
    t = threading.Thread(target=wait_reply, args=(rect))
    t.start()