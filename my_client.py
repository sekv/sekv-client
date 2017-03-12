#!/usr/bin/python 
import socket, sys

s=socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect(('localhost',11211))
data = 'set 0 0 0 1\r\n'
value = 'a\r\n'
s.sendall(data)
s.sendall(value)
print s.recv(1024)
