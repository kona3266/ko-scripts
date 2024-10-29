import zmq
import target_pb2
context = zmq.Context()
socket = context.socket(zmq.REP)
socket.bind("tcp://localhost:5555")

def handle_data(socket, message):
    recv_rec = target_pb2.Rect()
    recv_rec.ParseFromString(message)
    print(recv_rec.x1, recv_rec.y1)
    print(recv_rec.x2, recv_rec.y2)
    socket.send(b"new message")

def receive_data(socket):
    data = b''
    while True:
        try:
            data += socket.recv(1)
            size = decode_varint(data)
            break
        except IndexError:
            pass
    # Receive the message data
    message = socket.recv(size)
    return message

while True:
    message = socket.recv()
    t = threading.Thread(target=send_data, args=(socket, message))
    t.start()
