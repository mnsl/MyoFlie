import socket
import sys
import os

class Server:
    server_address = './uds_socket'
    connection = None 
    sock = None
    client_address = None

    def __init__(self):
        # make sure socket doesn't already exist
        try:
            os.unlink(self.server_address)
        except OSError:
            if os.path.exists(self.server_address): raise

        self.sock = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)

        # bind socket to port
        print >>sys.stderr, 'starting up on %s' % self.server_address
        self.sock.bind(self.server_address)

        # listen for incoming connections
        self.sock.listen(1)

    def send_data(self, data):
        print >>sys.stderr, 'data:', data
        try:
            if self.connection is None:
                print >>sys.stderr, 'waiting for a connection'
                self.connection, self.client_address = self.sock.accept()
                print >>sys.stderr, 'connection from', self.client_address
                return

            # send data in small chunks
            chunk_size = 16
            pos = 0
            print >>sys.stderr, 'sending data back to the client', data
            while len(data) - pos >= chunk_size:
                chunk = data[pos:pos+chunk_size]
                self.connection.sendall(chunk)
                pos += chunk_size
            self.connection.sendall(data[pos:])
        except socket.error:
            print >>sys.stderr, 'socket error; client has closed connection'
            self.connection = None
            self.client_address = None

    def close(self):
        if self.connection is not None:
            self.connection.close()

