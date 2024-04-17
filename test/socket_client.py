import socket
import sys
import argparse
import logging

if __name__ == '__main__':
    print('This is a test file for socket connection')

    str help_message = """
                            This is a test file for socket connection.\n
                            To use it, run as the following command:\n
                                python3 socket_connect.py --target_address <target_address:port_number>\n
                        """

    parser = argparse.ArgumentParser(description='Socket connection test')
    parser.add_argument('--target_address', action='store', dest='target', type=str, required=True)
    parser.add_argument('--help', action='help', help='')

    args = parser.parse_args()

    ip_address = args.target.split(':')[0]
    port_number = int(args.target.split(':')[1])

    # Create a TCP/IP socket
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    try:
        sock.connect(ip_address, port_number)
        print('Connection established')
        sock.sendall(b'Gimme some data')
        data = str(sock.recv(1024))
        print('Received:', data)
    except TimeoutError:
        print('Connection timeout')
        sys.exit(1)
    except InterruptedError:
        print('Connection interrupted')
        sys.exit(1)
    except Exception as e:
        print('Connection failed')
        print(e)
        sys.exit(1)
    
    

    sys.exit(0)