import socket
import json
from time import sleep
from typing import Callable, Optional

# TODO(Websocket) this test should be moved to Cool. And osc / http tests too probably


class Coollab:
    _host: str
    _port: int
    _s: Optional[socket.socket]

    def __init__(self, host: str = "127.0.0.1", port: int = 12345) -> None:
        self._host = host
        self._port = port
        self._s = None

    def __enter__(self) -> "Coollab":
        self._s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self._s.connect((self._host, self._port))
        return self

    def __exit__(self, exc_type, exc_value, traceback) -> None:
        if self._s:
            self._s.close()

    def _encode_json(self, dic: dict) -> bytes:
        return json.dumps(dic).encode("utf-8") + b"\0"

    def export_image(self, width: int = 500, height: int = 500) -> None:
        if self._s:
            self._s.sendall(
                self._encode_json(
                    {
                        "command": "ExportImage",
                        "width": width,
                        "height": height,
                        "format": ".png",
                    }
                )
            )

    def log(self, title: str, content: str) -> None:
        if self._s:
            self._s.sendall(
                self._encode_json(
                    {
                        "command": "Log",
                        "title": title,
                        "content": content,
                    }
                )
            )

    def close_app(self) -> None:
        if self._s:
            self._s.sendall(
                self._encode_json(
                    {
                        "command": "CloseApp",
                        "force_kill_task_in_progress": False,
                    }
                )
            )

    def wait_message(self) -> None:
        if not self._s:
            return
        data = bytearray()
        while True:
            chunk = self._s.recv(1024)
            if not chunk:
                break
            data += chunk
            if b"\0" in chunk:
                data = data.split(b"\0")[0]
                break
        print(data.decode())
        d = json.loads(data.decode())
        if d["event"] == "ImageExportFinished":
            self.callback()

    def on_image_export_finished(self, callback: Callable[[], None]) -> None:
        self.callback = callback


# Usage
with Coollab() as coollab:
    IMAGE_MAX = 10
    image_count = 0

    def increase_image_count():
        global image_count
        image_count += 1
        print(image_count)
        if image_count == IMAGE_MAX:
            coollab.close_app()

    coollab.on_image_export_finished(increase_image_count)
    for i in range(IMAGE_MAX):
        coollab.export_image(width=500, height=500)
        coollab.wait_message()
        # sleep(0.5)
    # for i in range(IMAGE_MAX):
    #     coollab.wait_message()
