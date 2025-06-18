import socket
import json

# TODO(Websocket) should be moved to Cool. And osc / http tests too probably

HOST = "127.0.0.1"  # or 'localhost'
PORT = 12345

from time import sleep


def encode_json(dic: dict):
    return json.dumps(dic).encode("utf-8") + b"\0"


with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.connect((HOST, PORT))
    i = 0
    while True:
        i += 1
        if i % 2 == 0:
            s.sendall(
                encode_json(
                    {
                        "command": "ExportImage",
                        "width": 1024,
                        "height": 1024,
                        "format": ".png",
                    }
                )
            )
        else:
            s.sendall(
                encode_json(
                    {
                        "command": "Log",
                        "title": "Scripting",
                        "content": f"This is a script! {i}",
                    }
                )
            )
        # s.sendall(b"Hello Server\0")
        sleep(1)
    # s.sendall(b"Server0")
    data = s.recv(1024)

print(f"Received: {data.decode()}")
