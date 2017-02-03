import socket
import sys
import os

server_address = './uds_socket'

# make sure socket doesn't already exist
try:
    os.unlink(server_address)
except OSError:
    if os.path.exists(server_address): raise

sock = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)

# bind socket ot port
print >>sys.stderr, 'starting up on %s' % server_address
sock.bind(server_address)

# listen for incoming connections
sock.listen(1)

while True:
    # wait for connection
    print >>sys.stderr, 'waiting for a connection'
    connection, client_address = sock.accept()
    try:
        print >>sys.stderr, 'connection from', client_address

        while True:
            # receive data in small chunks & retransmit it
            data = connection.recv(16)
            print >>sys.stderr, 'received "%s"' % data
            if data:
                print >>sys.stderr, 'sending data back to the client'
                connection.sendall(data)
            else:
                print >>sys.stderr, 'no more data from', client_address
                break
    finally:
        connection.close()

            
