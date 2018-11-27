import random
import socket
import sys

server = ("0.0.0.0", 8000)
clients = [("192.168.0.18", 8000), ("192.168.0.233", 8000)]
loss_rate = 0

argc = len(sys.argv)

if argc == 2:
    loss_rate = sys.argv[1]

print "Loss rate:", loss_rate

# create the socket
s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
s.bind(server)

# infinie run loop
while (True):
    # read a packet
    payload, incoming_client = s.recvfrom(1504)
    print "Incoming packet from", incoming_client

    # if packet was not randomly droppped
    if float(random.random()) > float(loss_rate):
        # send packet to correct host
        if incoming_client == clients[0]:
            s.sendto(payload, clients[1])
            print "Forwarding to", clients[1]
        elif incoming_client == clients[1]:
            s.sendto(payload, clients[0])
            print "Forwarding to", clients[0]
        else:
            print "Address error"
