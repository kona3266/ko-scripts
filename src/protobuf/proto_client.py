import target_pb2
import time
import os
import zmq
import threading
from google.protobuf.internal.encoder import _VarintEncoder
from google.protobuf.internal.decoder import _DecodeVarint

rect = target_pb2.Rect()
rect.x1 = 0
rect.y1 = 2
rect.x2 = 2
rect.y2 = 0
url = "tcp://localhost:5555"

'''
ZeroMQ sockets carry messages, like UDP, rather than a stream of bytes as TCP does. A ZeroMQ message is length-specified binary data. 
'''

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
        self.socket.send(self.data)

    def receive(self):
        # Receive the message data
        resp = self.socket.recv()
        print(resp)

def send_data():
    context = zmq.Context()
    data = rect.SerializeToString()
    client = Client(context, data)
    client.send(url)
    client.receive()

for i in range(4):
    t = threading.Thread(target=send_data, args=())
    t.start()
