import socket

HOST = "127.0.0.1"  # or 'localhost'
PORT = 12345


def encode_json(dic: dict):
    pass


with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.connect((HOST, PORT))
    # s.sendall(b"Hello Server")
    command = {
        "command": "ExportImage",
        "width": 1024,
        "height": 1024,
        "format": ".png",
    }
    s.sendall(encode_json(command))
    # s.sendall(b"Server0")
    data = s.recv(1024)

print(f"Received: {data.decode()}")
