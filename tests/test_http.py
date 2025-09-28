from math import sin
import requests

if __name__ == "__main__":
    for x in range(0, 1000000000):
        print(f"Sending {x}")
        requests.get(
            f"http://localhost:1234/set?name=my_name&value={sin(x / 100) * 0.5 + 0.5}"
        )
