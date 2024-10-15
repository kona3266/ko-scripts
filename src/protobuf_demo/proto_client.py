import target_pb2
import time
import os
import zmq

rect = target_pb2.Rect()
rect.x1 = 0
rect.y1 = 2
rect.x2 = 2
rect.y2 = 0

bytesdata = rect.SerializeToString()
context = zmq.Context()
socket = context.socket(zmq.REQ)
socket.connect("tcp://localhost:5555")
socket.send(bytesdata)
resp = socket.recv()
print(resp)
