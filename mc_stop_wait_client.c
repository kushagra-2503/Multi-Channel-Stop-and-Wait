#include "mc_stop_wait.h"

Packet buff_packet1;
Packet buff_packet2;
int timer_running;
int timer_channel;
int buff_socket1;
int buff_socket2;
static void handle_alarm(int sig)
{
    
    if(timer_running && timer_channel == 0)
    {
        //printf("Handling interrupt for packet1\n");
        write(buff_socket1, &buff_packet1, sizeof(Packet));
        printf("SENT PKT: Seq no. %d of size %d Bytes from channel 0\n", buff_packet1.seq_no, buff_packet1.size);
        timer_channel = 0;
        timer_running = 1;
        alarm(0);
        alarm(2);
    }
    else
    {
        if(timer_running)
        {
            //printf("Handling interrupt\n");
            //printf("%s\n", buff_packet2.data);
            write(buff_socket2, &buff_packet2, sizeof(Packet));
            printf("SENT PKT: Seq no. %d of size %d Bytes from channel 1\n", buff_packet2.seq_no, buff_packet2.size);
            timer_channel = 1;
            timer_running = 1;
            alarm(0);
            alarm(2);
        }
    }
    if(!timer_running)
    {
        alarm(0);
    }
}
int createSocket()
{
    int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(sock < 0)
    {
        printf("Error while creating socket\n");
        exit(1);
    }
    printf("Client Socket created succesfully: Socket - %d\n", sock);
    return sock;
}


int main()
{
    struct sockaddr_in serverAddress1, serverAddress2;
    signal(SIGALRM, handle_alarm);
    int sock1 = createSocket();
    int reuse = 1;
    setsockopt(sock1, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
    setsockopt(sock1, SOL_SOCKET, SO_REUSEPORT, (const char*)&reuse, sizeof(reuse));
    int sock2 = createSocket();
    setsockopt(sock1, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
    setsockopt(sock2, SOL_SOCKET, SO_REUSEPORT, (const char*)&reuse, sizeof(reuse));
    memset(&serverAddress1, 0, sizeof(serverAddress1));
    memset(&serverAddress2, 0, sizeof(serverAddress2));


    fd_set fds1, fds2;
   
    serverAddress1.sin_addr.s_addr = inet_addr("127.0.0.1");
    serverAddress1.sin_family = AF_INET;
    serverAddress1.sin_port = htons(5001);
    
    printf("Address assigned\n");
    int bytesReceived = 0;
    int c = connect(sock1, (struct sockaddr*) &serverAddress1, sizeof(serverAddress1));
    fcntl(sock1, F_SETFL,  O_NONBLOCK);
    if(c < 0)
    {
        printf("Connection Failed\n");
        exit(1);
    }
    printf("Connection Established for first channel\n");

    c = connect(sock2, (struct sockaddr*) &serverAddress1, sizeof(serverAddress1));
    fcntl(sock2, F_SETFL, O_NONBLOCK);
    if(c < 0)
    {
        printf("Connection Failed\n");
        exit(1);
    }
    printf("Connection Established for second channel\n");

    FILE* fp = fopen("input.txt", "rb");
    int offset = 0;
    char buff1[PACKET_SIZE + 1];
    char buff2[PACKET_SIZE + 1];
    int state = 0;
    int wait_channel_1 = 0;
    int wait_channel_2 = 0;
    Packet p1, p2, temp1, temp2;
    p1.channel_id = 0;
    p2.channel_id = 1;
    int bytesReceived1, bytesReceived2;
    int ended = 0;
    Packet p1_ack, p2_ack;
    int nread;
    buff_socket1 = sock1;
    buff_socket2 = sock2;
    int file_ended = 0;
    do
    {
        
        switch(state)
        {
            case 0: 
            if(wait_channel_1 == 0)
            {
                p1.seq_no = ftell(fp);
                //printf("p1: %d\n", p1.seq_no);
                memset(buff1, 0, sizeof(buff1));
                nread = 0;
                if(!file_ended)
                
                nread = fread(p1.data, 1, PACKET_SIZE, fp);
                if(nread > 0)
                {
                    p1.size = nread;
                    if(nread < PACKET_SIZE)
                    {
                        p1.end = 1;
                    }
                    else
                    {
                        p1.end = 0;
                    }
                    temp1 = p1;
                    buff_packet1 = p1;
                    write(sock1, (void *) &p1, sizeof(Packet));
                    printf("SENT PKT: Seq no. %d of size %d Bytes from channel 0\n", buff_packet1.seq_no, buff_packet1.size);
                    if(!timer_running)
                    {
                        timer_running = 1;
                        alarm(0);
                        alarm(2);
                        timer_channel = 0;
                    }
                    wait_channel_1 = 1;
                }
                else
                {
                    if(feof(fp) && wait_channel_2 == 0 && wait_channel_1 == 0)
                    {
                        file_ended  =1;
                        state = 2;
                        break;
                    }
                    if(ferror(fp))
                    {
                        printf("Error while reading\n");
                        ended = 1;
                    }
                }
            }
            state = 1;
            
            break;

            case 1:
            if(wait_channel_2 == 0)
            {
                p2.seq_no = ftell(fp);
                //printf("p2: %d\n", p2.seq_no);
                memset(p2.data, 0, sizeof(buff1));
                nread = 0;
                if(!file_ended)
                    nread = fread(p2.data, 1, PACKET_SIZE, fp);
                if(nread > 0)
                {
                    p2.size = nread;
                    if(nread < PACKET_SIZE)
                    {
                        p2.end = 1;
                    }
                    else
                    {
                        p2.end = 0;
                    }
                    temp2 = p2;
                    buff_packet2 = p2;
                    write(sock2, (void *) &p2, sizeof(Packet));
                    printf("SENT PKT: Seq no. %d of size %d Bytes from channel 1\n", buff_packet2.seq_no, buff_packet2.size);
                    if(!timer_running)
                    {
                        timer_running = 1;
                        alarm(0);
                        alarm(2);
                        timer_channel = 1;
                    }
                    wait_channel_2 = 1;
                }
                else
                {
                    if(feof(fp) && wait_channel_1 == 0 && wait_channel_2 == 0)
                    {
                        file_ended = 1;
                        state = 2;
                        break;
                    }
                    if(ferror(fp))
                    {
                        printf("Error while reading\n");
                        ended = 1;
                    }
                }
            }
            state = 2;
            break;

            case 2:
                if(feof(fp))
                {
                    ended = 1;
                    break;
                }
                while(wait_channel_2 == 1 && wait_channel_1 == 1)
                {
                        bytesReceived = read(sock1, &p1_ack, sizeof(p1));
                        
                        if(bytesReceived > 0)
                        {
                            //printf("Received ack for packet1\n");
                            printf("RCVD ACK: for PKT with seq no: %d, of from channel: 0\n", p1_ack.seq_no);
                            if(!timer_running && feof(fp))
                            {
                                ended = 1;
                                alarm(0);
                                break;
                            }
                            if(timer_running && timer_channel == 0)
                            {
                                if(wait_channel_2)
                                {
                                    timer_channel = 1;
                                    timer_running = 1;
                                    alarm(0);
                                    alarm(2);
                                }
                                else
                                {
                                    timer_running = 0;
                                    alarm(0);
                                }
                            
                            }
                            wait_channel_1 = 0;
                            state = 0;
                        }
                        bytesReceived = read(sock2, &p2_ack, sizeof(p2));
                        if(bytesReceived > 0)
                        {
                            //printf("Recieved ack for packet2:\n");
                            printf("RCVD ACK: for PKT with seq no: %d, of from channel: 1\n", p2_ack.seq_no);
                            if( feof(fp))
                            {
                                ended = 1;
                                alarm(0);
                                break;
                            }
                            if(timer_running && timer_channel == 1)
                            {
                                if(wait_channel_1)
                                {
                                    timer_channel = 0;
                                    timer_running = 1;
                                    alarm(0);
                                    alarm(2);
                                }
                                else
                                {
                                    timer_running = 0;
                                    alarm(0);
                                }
                                
                            }
                            wait_channel_2 = 0;
                            state = 0;
                        }
                    
                }
            break;
            
        }
    }while(ended == 0); 


    while(timer_running)
    {
        if(timer_channel == 0) 
        {
            int nread = read(sock1, &p1_ack, sizeof(p1));
            if(nread > 0)
            {
                //printf("Received ack for packet1:\n");
                printf("RCVD ACK: for PKT with seq no: %d, of from channel: 0\n", p1_ack.seq_no);
                alarm(0);
                timer_running = 0;
            }
        }
        else
        {
            int nread = read(sock2, &p2_ack, sizeof(p2));
            if(nread > 0)
            {
                //printf("Received ack for packet2:\n");
                printf("RCVD ACK: for PKT with seq no: %d, of from channel: 1\n", p2_ack.seq_no);
                alarm(0);
                timer_running = 0;
            }
        }
        
    }
    fclose(fp);
}