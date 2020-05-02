Multi Channel Stop and Wait Protocol

Execution Instructions:

1. Compile the files using command
     gcc -o client mc_stop_wait_client.c
     gcc -o server mc_stop_wait_server.c

2. Execution:
Please note first the server needs to be started using the command
    ./server
followed by the client with command
    ./client

Solution
    1. The TCP Connection between client and server used read and write system calls to transfer data.

    2. The two sockets were made non-blocking using fcntl function to handle concurrent transfers. I kept track of the
    return value from recv function and processed the result appropriately when it was positive. A negative or zero value
    meant the channel was waiting and data transfer could continue at the other channel. A 'wait_channel' flag was used for
    this purpose.

    3. Each channel required an independent timer, however since only one hardware timer is available, an approach similar to 
        Go-Back N Protocol was followed:
        Whenever a new packet is sent over a channel, first check whether the timer is running. If no, then start the timer,
        else do nothing. A flag was kept to keep track of the channel for which the timer was running. On reception of ack for a 
        packet, I first check if the timer was running for that particular packet (or that  channel equivalently since this is a
        stop and wait protocol). If yes, and a packet was there on the other channel, restart the timer for that channel.
        For retransmission, I first check for which channel the timer was running and re-transmit the packet accordingly.
    4. A seperate buffer was kept at the server for each channel. The in-order delivery was ensured by buffering upto 4 packets at the server and using mergeAndWrite() function which is
        called when 4 in-order packets are available. The function uses a principle similar to merge function for two sorted arrays.
        It is already confirmed that each buffer will be sorted in itself on sequence number, since a higher sequence number packet is not sent on the buffer unless
        the lower sequence number is acked.

Assumptions:
    The server terminates after serving one client.
    The file provided for input is similar to input.txt in terms of size and packet size.
    The server cannot handle multiple clients at the same time.
     Timeout has been assumed to be 2 seconds.
