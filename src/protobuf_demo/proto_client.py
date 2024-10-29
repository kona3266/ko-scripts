import target_pb2
import time
import os
import zmq
from google.protobuf.internal.encoder import _VarintEncoder
from google.protobuf.internal.decoder import _DecodeVarint

rect = target_pb2.Rect()
rect.x1 = 0
rect.y1 = 2
rect.x2 = 2
rect.y2 = 0
url = "tcp://localhost:5555"

def encode_varint(value):
    """ Encode an int as a protobuf varint """
    data = []
    _VarintEncoder()(data.append, value, False)
    return b''.join(data)


def decode_varint(data):
    """ Decode a protobuf varint to an int """
    return _DecodeVarint(data, 0)[0]

class Client():
    def __init__(self, ctx, data):
        context = ctx
        self.data = data
        self.socket = context.socket(zmq.REQ)

    def send(self, url):
        self.socket.connect(url)
        size = encode_varint(len(self.data))
        self.socket.send(size + self.data)

    def receive(self):
        data = b''
        while True:
            try:
                data += sef.socket.recv(1)
                size = decode_varint(data)
                break
            except IndexError:
                pass
        # Receive the message data
        resp = self.socket.recv(size)
        print(resp)

def send_data(data):
    context = zmq.Context()
    data = rect.SerializeToString()
    client = Client(context, data)
    client.send(url)
    client.receive()

for i in range(10):
    t = threading.Thread(target=send_data, args=(data))
    t.start()
