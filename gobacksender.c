/* 
   Go-Back-N Sender (UDP)
   Usage:
      ./gbn_sender <receiver-ip> <receiver-port> <window_size> <loss_prob> <timeout_ms>

   Example:
      ./gbn_sender 127.0.0.1 9000 4 0.3 2000
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <time.h>

#define MAXDATA 512
#define MAXFRAME 1024

double loss_prob;
int window_size;
int timeout_ms;

/* Random packet loss simulator */
int maybe_drop() {
    double r = (double)rand() / RAND_MAX;
    return (r < loss_prob);
}

/* Simple checksum */
unsigned char checksum(const unsigned char *buf, int len) {
    unsigned int s = 0;
    for (int i = 0; i < len; i++) s += buf[i];
    return s & 0xFF;
}

int main(int argc, char *argv[]) {
    if (argc < 6) {
        printf("Usage: %s <recv-ip> <recv-port> <window> <loss_prob> <timeout_ms>\n", argv[0]);
        return 1;
    }

    const char *ip = argv[1];
    int port = atoi(argv[2]);
    window_size = atoi(argv[3]);
    loss_prob = atof(argv[4]);
    timeout_ms = atoi(argv[5]);

    srand(time(NULL));

    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) { perror("socket"); return 1; }

    struct sockaddr_in serv;
    memset(&serv, 0, sizeof(serv));
    serv.sin_family = AF_INET;
    serv.sin_port = htons(port);
    inet_pton(AF_INET, ip, &serv.sin_addr);

    printf("GBN Sender Started (window=%d)\n", window_size);
    printf("Type messages. Ctrl+C to stop.\n");

    char line[1024];
    unsigned char frames[100][MAXFRAME];
    int frame_len[100];

    int base = 0;
    int nextseq = 0;

    while (1) {
        printf("Message: ");
        if (!fgets(line, sizeof(line), stdin)) break;

        int msglen = strlen(line);
        if (line[msglen - 1] == '\n') line[msglen - 1] = '\0';

        /* Create new frame */
        unsigned char frame[MAXFRAME];
        frame[0] = nextseq;  // sequence
        frame[1] = msglen;

        memcpy(&frame[2], line, msglen);

        frame[2 + msglen] = checksum(frame, 2 + msglen);
        int flen = 3 + msglen;

        memcpy(frames[nextseq], frame, flen);
        frame_len[nextseq] = flen;

        /* Send if within window */
        if (nextseq < base + window_size) {
            if (maybe_drop())
                printf("[SENDER] DROPPED Frame %d\n", nextseq);
            else {
                sendto(sockfd, frame, flen, 0, (struct sockaddr*)&serv, sizeof(serv));
                printf("[SENDER] Sent Frame %d\n", nextseq);
            }
            nextseq++;
        } else {
            printf("[SENDER] Window full – waiting for ACK...\n");
        }

        /* Wait for ACK or timeout */
        while (1) {
            fd_set r;
            FD_ZERO(&r);
            FD_SET(sockfd, &r);
            struct timeval tv;
            tv.tv_sec = timeout_ms / 1000;
            tv.tv_usec = (timeout_ms % 1000) * 1000;

            int rv = select(sockfd + 1, &r, NULL, NULL, &tv);

            if (rv == 0) {
                printf("[SENDER] TIMEOUT — Resending window [%d..%d)\n", base, nextseq - 1);
                for (int i = base; i < nextseq; i++) {
                    if (maybe_drop())
                        printf("[SENDER] DROPPED Frame %d\n", i);
                    else {
                        sendto(sockfd, frames[i], frame_len[i], 0,
                               (struct sockaddr*)&serv, sizeof(serv));
                        printf("[SENDER] Retransmitted Frame %d\n", i);
                    }
                }
            } 
            else if (rv > 0) {
                unsigned char ackbuf[10];
                socklen_t slen = sizeof(serv);
                int n = recvfrom(sockfd, ackbuf, sizeof(ackbuf), 0,
                                 (struct sockaddr*)&serv, &slen);

                int ackno = ackbuf[0];
                printf("[SENDER] Received ACK %d\n", ackno);

                base = ackno + 1;
                break;
            }
        }
    }

    close(sockfd);
    return 0;
}
