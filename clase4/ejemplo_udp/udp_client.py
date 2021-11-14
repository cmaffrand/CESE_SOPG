import socket

message = "Mensaje desde cliente"

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

sock.sendto(message.encode(), ("localhost",4096))

sock.close()

