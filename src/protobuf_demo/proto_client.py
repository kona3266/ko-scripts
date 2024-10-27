import target_pb2
import time
import os
import zmq

rect = target_pb2.Rect()
rect.x1 = 0
rect.y1 = 2
rect.x2 = 2
rect.y2 = 0
url = "tcp://localhost:5555"
class Client():
    def __init__(self, data):
        context = zmq.Context()
        self.data = data
        self.socket = context.socket(zmq.REQ)

    def send(self, url):
        self.socket.connect(url)
        self.socket.send(self.data)

    def receive(self):
        resp = self.socket.recv()
        print(resp)

def send_data(data):
    data = rect.SerializeToString()
    client = Client(data)
    client.send(url)
    client.receive()

for i in range(10):
    t = threading.Thread(target=send_data, args=(data))
    t.start()
