import zmq

context = zmq.Context()

# Socket to receive messages on
socket = context.socket(zmq.PULL)
socket.connect("tcp://wingpc.freeddns.org:25565")

while True:
    message = socket.recv_string()
    print(f"Received: {message}")
