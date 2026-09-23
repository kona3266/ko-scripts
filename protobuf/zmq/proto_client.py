import target_pb2
import zmq
import threading

rect = target_pb2.Rect()
rect.x1 = 0
rect.y1 = 2
rect.x2 = 2
rect.y2 = 0
url = "tcp://localhost:5555"

'''
ZeroMQ sockets carry messages, like UDP, rather than a stream of bytes as TCP does. A ZeroMQ message is length-specified binary data. 
'''

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
