#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include  <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <time.h>
#include <fcntl.h>
#include <sys/select.h>
#include <poll.h>
#include <signal.h>
#define PACKET_SIZE 100
#define PDR 1 /* For a drop rate of 10%, PDR is 1*/

struct packet{
    char data[PACKET_SIZE]; //payload of the packet. 
    int size; //Payload size in the packet.
    int seq_no; //Offset of the first byte of the packet.
    int end:2; //Whether last packet.
    int isData:2; //Ack or Data packet
    int channel_id:2; //Channel used for the packet.
};

typedef struct packet Packet;