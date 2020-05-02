#include "mc_stop_wait.h"


int createSocket()
{
    int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(sock < 0)
    {
        printf("Error while creating socket\n");
        exit(1);
    }
    printf("Server Socket created succesfully: Socket - %d\n", sock);
    return sock;
}

void mergeAndWrite(FILE* fp, Packet buff1[], int num_packets1, Packet buff2[], int num_packets2)
{
    int i = 0;
    int j = 0;
    while(i < num_packets1 && j < num_packets2)
    {
        if(buff1[i].seq_no < buff2[j].seq_no)
        {
            fwrite(buff1[i].data, 1, buff1[i].size, fp);
            fflush(fp);
            i++;
        }
        else
        {
            fwrite(buff2[j].data, 1, buff2[j].size, fp);
            fflush(fp);
            j++;
        }
    }

    while(i < num_packets1)
    {
        fwrite(buff1[i].data, 1, buff1[i].size, fp);
        fflush(fp);
        i++;
    }

    while(j < num_packets2)
    {
        fwrite(buff2[j].data, 1,buff2[j].size, fp);
        fflush(fp);
        j++;
    }
}

int main()
{
    //srand(time(NULL));

    int sockfd1;
    sockfd1 = createSocket();
    struct sockaddr_in serverAddress1, clientAddress1, clientAddress2;

    memset(&serverAddress1, 0, sizeof(serverAddress1));

    serverAddress1.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddress1.sin_family = AF_INET;
    serverAddress1.sin_port = htons(5001);
    int reuse = 1;
    setsockopt(sockfd1, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(int));
    setsockopt(sockfd1, SOL_SOCKET, SO_REUSEPORT, (const char*)&reuse, sizeof(reuse));
    bind(sockfd1, (struct sockaddr*) &serverAddress1, sizeof(serverAddress1));

    if(listen(sockfd1, 10) == -1)
    {
        printf("Listen Failed\n");
        exit(1);
    }
    else
    {
        printf("Listen1 success\n");
    }

    int clen;
    int conn1fd = accept(sockfd1, (struct sockaddr*) &clientAddress1, &clen);
    setsockopt(conn1fd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));
    setsockopt(conn1fd, SOL_SOCKET, SO_REUSEPORT, (const char*)&reuse, sizeof(reuse));
    int conn2fd = accept(sockfd1, (struct sockaddr*) &clientAddress2, &clen);
    setsockopt(conn2fd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));
    setsockopt(conn2fd, SOL_SOCKET, SO_REUSEPORT, (const char*)&reuse, sizeof(reuse));
    // int flags = fcntl(conn1fd, F_GETFL, 0);
    fcntl(conn1fd, F_SETFL,  O_NONBLOCK);
    // flags = fcntl(conn2fd, F_GETFL, 0);
    fcntl(conn2fd, F_SETFL, O_NONBLOCK);

    FILE* fp = fopen("output.txt", "wb");
    
    Packet buff1[1000];
    Packet buff2[1000];
    Packet ack_packet1, ack_packet2;
    int num_packets1, num_packets2, total_packets;
    num_packets1 = num_packets2 = total_packets = 0;
    memset(buff1, 0, sizeof(buff1));
    memset(buff2, 0, sizeof(buff2));
    fd_set fds1, fds2;
    int pending1 = 0;
    int pending2  = 0;

    while(1)
    {
        
        
        FD_ZERO(&fds1);
        FD_SET(conn1fd, &fds1);
        FD_ZERO(&fds2);
        FD_SET(conn2fd, &fds2);
        int nread1, nread2;
        nread1 = 0; nread2 = 0;
            nread1 = read(conn1fd, &buff1[num_packets1], sizeof(Packet));
            if(nread1 > 0)
            {
            //printf("Recieved packet1: %s\n", buff1[num_packets1].data);
            ack_packet1.channel_id = 0;
            ack_packet1.size = 0;
            ack_packet1.seq_no = buff1[num_packets1].seq_no;
            ack_packet1.isData = 0;
            //strcpy(ack_packet1.data, buff1[num_packets1].data);
            printf("channel1 : %d\n", buff1[num_packets1].seq_no);
            
            if(rand() % 10 >= PDR)
            {
                printf("RCVD PKT: SEQ no.- %d of size %d bytes from channel 0\n", buff1[num_packets1].seq_no, buff1[num_packets1].size);
                write(conn1fd, &ack_packet1, sizeof(ack_packet1));
                printf("SENT ACK FOR PKT with seq no: %d from channel 0\n", ack_packet1.seq_no);
                pending1 = 0;
                num_packets1++;
                total_packets++;
                if((total_packets >= 4  && num_packets2 && num_packets1) ||buff1[num_packets1-1].end)
                {   
                    
                    if(buff1[num_packets1 -1].end)
                    {
                    
                        break;
                    }
                    mergeAndWrite(fp, buff1, num_packets1, buff2, num_packets2);
                    total_packets = 0;
                    num_packets1 = 0;
                    num_packets2 = 0;
                    memset(buff1, 0, sizeof(buff1));
                    memset(buff2, 0, sizeof(buff2));
                }
            }
            else
            {
                memset(&buff2[num_packets2], 0, sizeof(Packet));
                pending1 = 1;
                printf("Dropping packet\n");
            }
            
            //}
        }
        //if(select(5, &fds2, NULL, NULL, 0) == 1)
        //{
            nread2 = read(conn2fd, &buff2[num_packets2], sizeof(Packet));
            //printf("Here\n");
        //printf("%d\n", nread1);
        
        if(nread2 > 0)
        {
            
            ack_packet2.channel_id = 0;
            ack_packet2.size = 0;
            ack_packet2.seq_no = buff2[num_packets2].seq_no;
            ack_packet2.isData = 0;
            
            printf("channel2 : %d\n", buff2[num_packets2].seq_no);
            
            if(rand() % 10 >= PDR)
            {
                printf("RCVD PKT: SEQ no.- %d of size %d bytes from channel 1\n", buff2[num_packets2].seq_no, buff2[num_packets2].size);
                write(conn2fd, &ack_packet2, sizeof(ack_packet2));
                printf("SENT ACK FOR PKT with seq no: %d from channel 1\n", ack_packet2.seq_no);
                pending2 = 0;
                num_packets2++;
                total_packets++;
                if((total_packets >= 4 && num_packets1 && num_packets2) ||  buff2[num_packets2-1].end)
                {
                   
                    if(buff2[num_packets2 -1].end)
                    {
                        break;
                        
                    }
                    mergeAndWrite(fp, buff1, num_packets1, buff2, num_packets2);
                    total_packets = 0;
                    num_packets2 = 0;
                    num_packets1 = 0;
                    memset(buff2, 0, sizeof(buff2));
                    memset(buff1, 0, sizeof(buff1));
                }
            }
            else
            {
                memset(&buff2[num_packets2], 0, sizeof(Packet));
                pending2  =1;
                printf("Dropping packet\n");
            }
            
        }
    }
    while(pending1 || pending2)
    {
        if(pending1)
        {
            //num_packets1 = 0;
            int nread1 =  read(conn1fd, &buff1[num_packets1], sizeof(Packet));
            if(nread1 > 0)
            {
                ack_packet1.channel_id = 0;
                ack_packet1.size = 0;
                ack_packet1.seq_no = buff1[num_packets1].seq_no;
                ack_packet1.isData = 0;
                strcpy(ack_packet1.data, buff1[num_packets1].data);
                //printf("channel1 : %d\n", buff1[num_packets1].seq_no);
                printf("RCVD PKT: SEQ no.- %d of size %d bytes from channel 0\n", buff1[num_packets1].seq_no, buff1[num_packets1].size);
                write(conn1fd, &ack_packet1, sizeof(ack_packet1));
                printf("SENT ACK FOR PKT with seq no: %d from channel 0\n", ack_packet1.seq_no);
                pending1 = 0;
                num_packets1++;
                total_packets++;
            }
        }
        if(pending2)
        {
            //num_packets2  =0;
            int nread2 =  read(conn2fd, &buff2[num_packets2], sizeof(Packet));
            if(nread2 > 0)
            {
                ack_packet2.channel_id = 0;
                ack_packet2.size = 0;
                ack_packet2.seq_no = buff1[num_packets1].seq_no;
                ack_packet2.isData = 0;
                strcpy(ack_packet2.data, buff2[num_packets1].data);
                //printf("channel2 : %d\n", buff2[num_packets2].seq_no);
                printf("RCVD PKT: SEQ no.- %d of size %d bytes from channel 1\n", buff2[num_packets2].seq_no, buff2[num_packets2].size);
                write(conn2fd, &ack_packet2, sizeof(ack_packet2));
                printf("SENT ACK FOR PKT with seq no: %d from channel 1\n", ack_packet2.seq_no);
                pending2 = 0;
                num_packets2++;
                total_packets++;
                //mergeAndWrite(fp, buff1, num_packets1, buff2, num_packets2);
            }
        }
    }

    mergeAndWrite(fp, buff1, num_packets1, buff2, num_packets2);
    
    while(pending2)
    {
        num_packets2  =0;
        int nread2 =  read(conn2fd, &buff2[num_packets2], sizeof(Packet));
        if(nread2 > 0)
        {
            ack_packet2.channel_id = 0;
            ack_packet2.size = 0;
            ack_packet2.seq_no = buff1[num_packets1].seq_no;
            ack_packet2.isData = 0;
            strcpy(ack_packet2.data, buff2[num_packets1].data);
            //printf("channel1 : %d\n", buff1[num_packets1].seq_no);
            write(conn2fd, &ack_packet1, sizeof(ack_packet1));
            pending2 = 0;
            num_packets2++;
            total_packets++;
            //mergeAndWrite(fp, buff1, num_packets1, buff2, num_packets2);
        }
    }
    
    
    fclose(fp);
}