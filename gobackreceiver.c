/*
   Go-Back-N Receiver (UDP)
   Usage:
      ./gbn_receiver <port> <loss_prob>

   Example:
      ./gbn_receiver 9000 0.2
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>

#define MAXFRAME 1024

double loss_prob;

/* Random ACK loss simulator */
int maybe_drop() {
    double r = (double)rand() / RAND_MAX;
    return (r < loss_prob);
}

/* Checksum */
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

    memset(&me, 0, sizeof(me));
    me.sin_family = AF_INET;
    me.sin_addr.s_addr = INADDR_ANY;
    me.sin_port = htons(port);

    bind(sockfd, (struct sockaddr*)&me, sizeof(me));

    printf("GBN Receiver Listening on %d\n", port);

    int expected = 0;

    while (1) {
        unsigned char buf[MAXFRAME];

        int n = recvfrom(sockfd, buf, sizeof(buf), 0,
                         (struct sockaddr*)&cli, &clen);

        int seq = buf[0];
        int len = buf[1];

        unsigned char ch = checksum(buf, 2 + len);

        if (buf[2 + len] != ch) {
            printf("[RECV] BAD checksum on frame %d — discarding\n", seq);
            continue;
        }

        if (seq == expected) {
            buf[2 + len] = 0;
            printf("[RECV] Got frame %d: %s\n", seq, &buf[2]);

            expected++;

            /* Send ACK */
            unsigned char ack = seq;

            if (maybe_drop())
                printf("[RECV] DROPPED ACK %d\n", ack);
            else {
                sendto(sockfd, &ack, 1, 0, (struct sockaddr*)&cli, clen);
                printf("[RECV] Sent ACK %d\n", ack);
            }
        }
        else {
            printf("[RECV] Out-of-order frame %d (expected %d) — resend ACK %d\n",
                   seq, expected, expected - 1);

            unsigned char ack = expected - 1;

            if (!maybe_drop()) 
                sendto(sockfd, &ack, 1, 0, (struct sockaddr*)&cli, clen);
        }
    }

    close(sockfd);
    return 0;
}
