#!/usr/bin/env python3
import sys
import socket
import argparse

if __name__ == '__main__':

    argparse = argparse.ArgumentParser()
    argparse.add_argument('--ip', type=str, default='192.168.0.15',
                        help='IP address of the server')
    argparse.add_argument('--port', type=int, default=8883, help='Port number of the server')
    args = argparse.parse_args()

    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server_address = (args.ip, args.port)
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
                if data == "REQACK":
                    print('Sending acknowledge to the client')
                    connection.sendall(b'ACK')
                elif data == b'':
                    print('client disconnected', client_address)
                    break
                else:
                    print('Unknown data received')
        finally:
            connection.close()