import zmq
import protobuf.zmq_socket.target_pb2 as target_pb2
import threading
context = zmq.Context()
socket = context.socket(zmq.REP)
socket.bind("tcp://localhost:5555")

'''
ZeroMQ sockets carry messages, like UDP, rather than a stream of bytes as TCP does. A ZeroMQ message is length-specified binary data.
'''

def handle_data(socket, message):
    print('handle msg')
    recv_rec = target_pb2.Rect()
    recv_rec.ParseFromString(message)
    print(recv_rec.x1, recv_rec.y1)
    print(recv_rec.x2, recv_rec.y2)
    socket.send(b"new message")

def receive_data(socket):
    message = socket.recv()
    return message

while True:
    print('waiting for connect')
    message = receive_data(socket)
    handle_data(socket, message)
