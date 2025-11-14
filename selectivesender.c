/*
   Selective Repeat Sender (UDP)
   Usage:
      ./sr_sender <receiver-ip> <receiver-port> <window_size> <loss_prob> <timeout_ms>
   Example:
      ./sr_sender 127.0.0.1 9000 4 0.2 2000
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <sys/select.h>
#include <time.h>

#define MAXDATA 512
#define MAXFRAME 1024
#define MAXWINDOW 10

typedef struct {
    unsigned char data[MAXFRAME];
    int len;
    int acked;
    struct timeval sent_time;
} Frame;

int window_size;
double loss_prob;
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

/* Get time difference in ms */
long time_diff_ms(struct timeval t1, struct timeval t2) {
    return (t1.tv_sec - t2.tv_sec)*1000 + (t1.tv_usec - t2.tv_usec)/1000;
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

    Frame window[MAXWINDOW];
    memset(window, 0, sizeof(window));
    int base = 0;
    int nextseq = 0;

    char line[1024];
    socklen_t servlen = sizeof(serv);

    printf("Selective Repeat Sender Started (window=%d)\n", window_size);

    while (1) {
        printf("Message: ");
        if (!fgets(line, sizeof(line), stdin)) break;

        int msglen = strlen(line);
        if (line[msglen-1]=='\n') { line[msglen-1]='\0'; msglen--; }

        if (nextseq >= base + window_size) {
            printf("[SENDER] Window full, waiting for ACK...\n");
            goto wait_for_ack;
        }

        /* Prepare frame */
        window[nextseq].data[0] = nextseq; // seq
        window[nextseq].data[1] = msglen;  // len
        memcpy(&window[nextseq].data[2], line, msglen);
        window[nextseq].data[2 + msglen] = checksum(window[nextseq].data, 2 + msglen);
        window[nextseq].len = 3 + msglen;
        window[nextseq].acked = 0;
        gettimeofday(&window[nextseq].sent_time, NULL);

        /* Send frame */
        if (maybe_drop())
            printf("[SENDER] DROPPED Frame %d\n", nextseq);
        else {
            sendto(sockfd, window[nextseq].data, window[nextseq].len, 0,
                   (struct sockaddr*)&serv, servlen);
            printf("[SENDER] Sent Frame %d\n", nextseq);
        }

        nextseq++;

wait_for_ack:
        /* Check for ACKs and timeout */
        while (1) {
            fd_set r;
            FD_ZERO(&r);
            FD_SET(sockfd, &r);
            struct timeval tv = {0, 100*1000}; // 100ms wait
            int rv = select(sockfd+1, &r, NULL, NULL, &tv);

            if (rv > 0) {
                unsigned char ackbuf[2];
                int n = recvfrom(sockfd, ackbuf, sizeof(ackbuf), 0,
                                 (struct sockaddr*)&serv, &servlen);
                int ackno = ackbuf[0];
                printf("[SENDER] Received ACK %d\n", ackno);
                if (ackno >= base && ackno < nextseq)
                    window[ackno].acked = 1;
            }

            /* Retransmit if timeout */
            struct timeval now;
            gettimeofday(&now, NULL);
            int all_acked = 1;
            for (int i = base; i < nextseq; i++) {
                if (!window[i].acked) {
                    all_acked = 0;
                    long diff = time_diff_ms(now, window[i].sent_time);
                    if (diff >= timeout_ms) {
                        if (maybe_drop())
                            printf("[SENDER] DROPPED Frame %d (timeout retransmission)\n", i);
                        else {
                            sendto(sockfd, window[i].data, window[i].len, 0,
                                   (struct sockaddr*)&serv, servlen);
                            printf("[SENDER] Retransmitted Frame %d\n", i);
                        }
                        gettimeofday(&window[i].sent_time, NULL);
                    }
                }
            }
            /* Slide window */
            while (base < nextseq && window[base].acked) base++;

            if (all_acked) break; // send next frame
        }
    }

    close(sockfd);
    return 0;
}
