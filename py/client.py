import socket
import sys

sock = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)

server_address = './uds_socket'
print >>sys.stderr, 'connecting to %s' % server_address
try:
    sock.connect(server_address)
except socket.err, msg:
    print >>sys.stderr, msg
    sys.exit(1)

try:
    # send data
    msg = 'this is the message; it will be repeated'
    print >>sys.stderr, 'sending "%s"' % msg
    sock.sendall(msg)

    amount_received = 0
    amount_expected = len(msg)

    while amount_received < amount_expected:
        data = sock.recv(16)
        amount_received += len(data)
        print >>sys.stderr, 'received "%s"' % data

finally:
    print >>sys.stderr, 'closing socket'
    sock.close()
