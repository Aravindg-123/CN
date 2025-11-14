/*
   Selective Repeat Receiver (UDP)
   Usage:
      ./sr_receiver <port> <loss_prob>
   Example:
      ./sr_receiver 9000 0.1
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>

#define MAXDATA 512
#define MAXWINDOW 10
#define MAXFRAME 1024

double loss_prob;

int maybe_drop() {
    double r = (double)rand() / RAND_MAX;
    return (r < loss_prob);
}

unsigned char checksum(const unsigned char *buf, int len) {
    unsigned int s = 0;
    for (int i = 0; i < len; i++) s += buf[i];
    return s & 0xFF;
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Usage: %s <port> <loss_prob>\n", argv[0]);
        return 1;
    }

    int port = atoi(argv[1]);
    loss_prob = atof(argv[2]);
    srand(time(NULL));

    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) { perror("socket"); return 1; }

    struct sockaddr_in me, cli;
    socklen_t clen = sizeof(cli);

    memset(&me,0,sizeof(me));
    me.sin_family = AF_INET;
    me.sin_addr.s_addr = INADDR_ANY;
    me.sin_port = htons(port);

    bind(sockfd, (struct sockaddr*)&me, sizeof(me));

    printf("Selective Repeat Receiver listening on %d\n", port);

    int expected = 0;
    unsigned char received[MAXWINDOW][MAXDATA+3];
    int recv_len[MAXWINDOW];
    int received_flag[MAXWINDOW];
    memset(received_flag, 0, sizeof(received_flag));

    while(1){
        unsigned char buf[MAXFRAME];
        int n = recvfrom(sockfd, buf, sizeof(buf),0,(struct sockaddr*)&cli,&clen);

        int seq = buf[0];
        int len = buf[1];

        unsigned char ch = checksum(buf, 2+len);
        if (buf[2+len] != ch) {
            printf("[RECV] Bad checksum on frame %d, discarding\n", seq);
            continue;
        }

        received[seq][0] = seq;
        memcpy(&received[seq][2], &buf[2], len);
        received[seq][1] = len;
        received[seq][2+len] = ch;
        received_flag[seq] = 1;

        /* Send ACK */
        unsigned char ack = seq;
        if (maybe_drop())
            printf("[RECV] DROPPED ACK %d\n", ack);
        else {
            sendto(sockfd, &ack, 1, 0, (struct sockaddr*)&cli, clen);
            printf("[RECV] Sent ACK %d\n", ack);
        }

        /* Deliver in-order */
        while(received_flag[expected]) {
            printf("[RECV] Delivered frame %d: %.*s\n",
                   expected, received[expected][1], &received[expected][2]);
            received_flag[expected] = 0;
            expected++;
        }
    }

    close(sockfd);
    return 0;
}
