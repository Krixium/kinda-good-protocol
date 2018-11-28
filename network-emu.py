from collections import deque
from threading import Thread
import random
import socket
import sys
import time

def incoming(s, q, d):
    # infinie run loop
    while True:
        # read a packet
        payload, incoming_client = s.recvfrom(1504)
        print "Incoming packet from", incoming_client
        # if the packet is not being droppped, queue it
        if float(random.random()) > float(loss_rate):
            q.append((payload, incoming_client, time.time() + int(d)))


def outgoing(s, q, clients):
    # infinite run loop
    while True:
        while len(q) > 0:
            x = q.popleft()
            payload = x[0]
            client = x[1]
            out_time = x[2]
            if time.time() > out_time:
                if client == clients[0]:
                    s.sendto(payload, clients[1])
                elif client == clients[1]:
                    s.sendto(payload, clients[0])
                else:
                    print "Error"
            else:
                q.appendleft(x)
                break


# variables
server = ("0.0.0.0", 8000)
clients = [("192.168.0.18", 8000), ("192.168.0.233", 8000)]
loss_rate = 0
delay = 0
packet_queue = deque()

# getting user input
argc = len(sys.argv)

if argc >= 2:
    loss_rate = sys.argv[1]

if argc >= 3:
    delay = sys.argv[2]


# start
print "Loss rate:", loss_rate

# create the socket
s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
s.bind(server)

incomingThread = Thread(target=incoming, args=(s, packet_queue, delay,))
outgoingThread = Thread(target=outgoing, args=(s, packet_queue, clients,))

incomingThread.start()
outgoingThread.start()

incomingThread.join()
outgoingThread.join()
