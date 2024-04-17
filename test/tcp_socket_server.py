#!/usr/bin/env python3
import sys
import socket

if __name__ == '__main__':
    print('This is a test file for socket server')

    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server_address = ('192.168.0.10', 8883)
    sock.bind(server_address)

    sock.listen(1)
    while True:
        print('Waiting for a connection')
        connection, client_address = sock.accept()
        try:
            print('Connection from', client_address)
            while True:
                data = connection.recv(1024).decode('utf-8')
                print('Received:', data)
                if data == 'REQACK':
                    print('Sending acknowledge to the client')
                    connection.sendall(b'ACK')
                elif data == b'':
                    print('client disconnected', client_address)
                    break
                else:
                    print('Unknown data received')
        finally:
            connection.close()