import zmq
import target_pb2
context = zmq.Context()
socket = context.socket(zmq.REP)
socket.bind("tcp://localhost:5555")

while True:
    message = socket.recv()
    recv_rec = target_pb2.Rect()
    recv_rec.ParseFromString(message)
    print(recv_rec.x1, recv_rec.y1)
    print(recv_rec.x2, recv_rec.y2)
    socket.send(b"new message")
