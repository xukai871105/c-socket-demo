import socket
import binascii

HOST = ''
PORT = 8018

s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
s.bind((HOST, PORT))

while True:
    request, client_address = s.recvfrom(1024)
    print('Connected by', client_address,
          'Received ', binascii.hexlify(request))
    if request:
        s.sendto(request, client_address)

