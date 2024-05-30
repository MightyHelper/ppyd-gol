import time

import zmq

context = zmq.Context()

socket = context.socket(zmq.PUSH)
socket.bind("tcp://0.0.0.0:25565")
for i in range(10):
    message = f"Message {i}"
    print(f"Sending: {message}")
    socket.send_string(message)
    time.sleep(1)  # simulate work
