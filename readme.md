# Multi Channel Stop and Wait Protocol

This repository contains an implementation of a stop and wait protocol over two concurrent channels for a dile transfer from client to server.

## Execution Instructions:

1. Compile the files using command
     gcc -o client mc_stop_wait_client.c
     gcc -o server mc_stop_wait_server.c

2. Execution:
Please note first the server needs to be started using the command
    ./server
followed by the client with command
    ./client
 
## Multi-channel stop-and-wait protocol without DATA or ACK losses.
1. The sender transmits packets through two different channels (TCP connections).
2. The server acknowledges the receipt of a packet via the same channel through which the
corresponding packet has been received.
3. The sender transmits a new packet using the same channel on which it has received an ACK for its
one of the previously transmitted packet. Note that, at a time, there can be at most two outstanding
unacknowledged packets at the sender side.
4. On the server-side, the packets transmitted through different channels may arrive out of order. In
that case, the server has to buffer the packets temporarily to finally construct in-order data stream.

Write client and server programs to upload a given file (“input.txt”) from client to the server using
ARQ protocol as described above by making TCP connections between the client and the server.

