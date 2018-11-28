#-------------------------------------------------------------------------------------
#-	SOURCE FILE:		    network-emu.py
#-
#-	PROGRAM:		        Network Emulator
#-
#-	FUNCTIONS:
#-                          control(socket)
#-                          incoming(socket, queue, delay):
#-                          outgoing(socket, queue, clients):
#-
#-
#-	DATE:			        November 27, 2018
#-
#-	REVISIONS:		        N/A
#-
#-	DESIGNERS:		        Benny Wang
#-
#-	PROGRAMMERS:		    Benny Wang
#-
#-	NOTES:
#-                          Fowards packets between two hosts. A loss rate and average
#-                          delay can be specificed in command line arguements in the form:
#-                              python3 network-emu.py [loss rate] [average delay]
#--------------------------------------------------------------------------------------
import collections
import threading
import random
import socket
import sys
import time


# variables
lock = threading.Lock()
running = True
port = 8000
server = ("0.0.0.0", port)
clients = [("192.168.0.12", port), ("192.168.0.233", port)]
loss_rate = 0
delay = 0
packet_queue = collections.deque()


#--------------------------------------------------------------------------------------------------
#- FUNCTION:                control
#-
#- DATE:                    November 27, 2018
#-
#- REVISIONS:               N/A
#-
#- DESIGNER:                Benny Wang
#-
#- PROGRAMMER:              Benny Wang
#-
#- INTERFACE:               void control(socket)
#-                              socket: A bound udp socket.
#-
#- NOTES:
#-                          Listens for input fron stdin. If "q" or "quit" is entered by the user
#-                          this thread stops execution of the program.
#-------------------------------------------------------------------------------------------------
def control(socket):
    global running
    global lock

    while running:
        buffer = input()
        buffer.lower()

        if buffer == "q" or buffer == "quit":
            print("Quitting ...")
            with lock:
                running = False
                socket.close()

    print("Control thread stopping")


#--------------------------------------------------------------------------------------------------
#- FUNCTION:                incoming
#-
#- DATE:                    November 27, 2018
#-
#- REVISIONS:               N/A
#-
#- DESIGNER:                Benny Wang
#-
#- PROGRAMMER:              Benny Wang
#-
#- INTERFACE:               void incoming(socket, queue, delay)
#-                              socket: A bound udp socket.
#-                              queue: A queue to hold incoming packets.
#-                              delay: The average delay to add to packets.
#-
#- NOTES:
#-                          Stores all incoming packets into a queue. The packet is stored with
#-                          its sender and the timestamp of when it should be forwarded.
#-------------------------------------------------------------------------------------------------
def incoming(socket, queue, delay):
    global running
    global lock

    # infinie run loop
    while running:
        # read a packet and sender
        payload, incoming_client = socket.recvfrom(1504)
        print("Incoming packet from", incoming_client)

        # check if this packet should be dropped
        if float(random.random()) > float(loss_rate):
            # queue it
            with lock:
                queue.append((payload, incoming_client, time.time() + float(delay)))
        else:
            print("Packet dropped")

    print("Incoming thread stopping")


#--------------------------------------------------------------------------------------------------
#- FUNCTION:                outgoing
#-
#- DATE:                    November 27, 2018
#-
#- REVISIONS:               N/A
#-
#- DESIGNER:                Benny Wang
#-
#- PROGRAMMER:              Benny Wang
#-
#- INTERFACE:               void outgoing(socket, queue, clients)
#-                              socket: A bound udp socket.
#-                              queue: The queue of packets to send.
#-                              clients: A list of valid clients.
#-
#- NOTES:
#-                          Constantly checks the queue to see if there are packets to forward.
#-------------------------------------------------------------------------------------------------
def outgoing(socket, queue, clients):
    global running
    global lock

    while running:
        # send all the packets that have waited long enough
        while len(queue) > 0:
            # get the first tuple in queue
            with lock:
                packet_tuple = queue.popleft()

            # alias to make code more readable
            payload = packet_tuple[0]
            client = packet_tuple[1]
            out_time = packet_tuple[2]

            # if it has waited long enough
            if time.time() > out_time:
                # send it
                if client == clients[0]:
                    socket.sendto(payload, clients[1])
                elif client == clients[1]:
                    socket.sendto(payload, clients[0])
                else:
                    print("Error")
            else:
                # put it back in the queue and continue
                with lock:
                    queue.appendleft(packet_tuple)

    print("Outgoing thread stopping")


# ============================== start of program ==============================
# getting user input from args
argc = len(sys.argv)

if argc >= 2:
    loss_rate = sys.argv[1]

if argc >= 3:
    delay = sys.argv[2]


# start
print("Loss rate:", loss_rate)
print("Average delay:", delay)

# create the socket
s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
s.bind(server)

controlThread = threading.Thread(target=control, args=(s,))
incomingThread = threading.Thread(target=incoming, args=(s, packet_queue, delay,))
outgoingThread = threading.Thread(target=outgoing, args=(s, packet_queue, clients,))

controlThread.start()
incomingThread.start()
outgoingThread.start()

controlThread.join()
incomingThread.join()
outgoingThread.join()
# ============================== end of program ==============================