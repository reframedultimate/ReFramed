import sys
import socket
import time


speed_multiplier = 2
mapping_info_checksum = 1177268349

filename = sys.argv[1]

server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
server.bind(("0.0.0.0", 42069))
server.listen(5)

client, addr = server.accept()
print("Send Version + Checksum")
client.send(b'\x00\x01\x00')
client.send(bytes([
    (mapping_info_checksum >> 24) & 0xFF,
    (mapping_info_checksum >> 16) & 0xFF,
    (mapping_info_checksum >> 8) & 0xFF,
    mapping_info_checksum & 0xFF
]))

with open(filename, "rb") as f:
    print("Sending data")
    while True:
        try:
            delay = f.read(1)[0]
            while delay != 0:
                time.sleep(delay * 0.001 / speed_multiplier)
                delay = f.read(1)[0]

            l = f.read(1)[0]
            blob = f.read(l)
            client.send(blob)
        except Exception:
            break

print("Done")
