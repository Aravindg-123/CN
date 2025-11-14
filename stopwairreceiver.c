/*
 stopwait_receiver.c
 Usage: ./stopwait_receiver <listen-port> <loss_prob>
 Example: ./stopwait_receiver 9000 0.1
 loss_prob = probability [0.0..1.0] that an outgoing ACK is dropped (for testing)
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>
#include <sys/socket.h>
#include <stdint.h>

#define MAXDATA 512
#define BUF_SIZE (1 + 2 + 1 + MAXDATA)

unsigned char checksum(const unsigned char *buf, int len) {
    unsigned int s = 0;
    for (int i = 0; i < len; ++i) s += buf[i];
    return (unsigned char)(s & 0xFF);
}

int maybe_drop(double loss_prob) {
    if (loss_prob <= 0.0) return 0;
    double r = (double)rand() / RAND_MAX;
    return (r < loss_prob);
}

int main(int argc, char **argv) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <listen-port> <loss_prob>\n", argv[0]);
        return 1;
    }

    int listen_port = atoi(argv[1]);
    double loss_prob = atof(argv[2]);
    srand((unsigned)time(NULL));

    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) { perror("socket"); return 1; }

    struct sockaddr_in myaddr, cliaddr;
    memset(&myaddr, 0, sizeof(myaddr));
    myaddr.sin_family = AF_INET;
    myaddr.sin_addr.s_addr = INADDR_ANY;
    myaddr.sin_port = htons(listen_port);

    if (bind(sockfd, (struct sockaddr *)&myaddr, sizeof(myaddr)) < 0) {
        perror("bind"); close(sockfd); return 1;
    }

    printf("Stop-and-Wait Receiver listening on port %d\n", listen_port);

    unsigned char expected_seq = 0;
    unsigned char last_ack_seq = 1; // none yet; set to opposite so duplicate handling works

    while (1) {
        unsigned char buf[BUF_SIZE];
        socklen_t len = sizeof(cliaddr);
        ssize_t r = recvfrom(sockfd, buf, sizeof(buf), 0,
                             (struct sockaddr *)&cliaddr, &len);
        if (r <= 0) { perror("recvfrom"); continue; }

        // parse frame: seq(1), len(2), chksum(1), data
        if (r < (1 + 2 + 1)) {
            printf("[RECV] Ignoring too-small frame (len %zd)\n", r);
            continue;
        }
        unsigned char seq = buf[0];
        uint16_t nlen_net;
        memcpy(&nlen_net, &buf[1], 2);
        uint16_t nlen = ntohs(nlen_net);
        if (nlen > MAXDATA) { printf("[RECV] Bad length %u, drop\n", nlen); continue; }
        if (r < 1 + 2 + 1 + nlen) {
            printf("[RECV] Incomplete frame (declared len %u, got %zd), drop\n", nlen, r);
            continue;
        }
        unsigned char recv_ch = buf[1+2];
        unsigned char calc = checksum(buf, 1 + 2 + nlen); // seq + len + data
        if (recv_ch != calc) {
            printf("[RECV] Checksum mismatch for seq=%u (dropping frame)\n", seq);
            // simulate no ACK on checksum error
            continue;
        }

        // If expected sequence -> deliver and ack
        if (seq == expected_seq) {
            // deliver
            char message[MAXDATA+1];
            memcpy(message, &buf[1+2+1], nlen);
            message[nlen] = '\0';
            printf("[RECV] Received NEW frame seq=%u: \"%s\"\n", seq, message);

            // send ACK
            unsigned char ack[3];
            ack[0] = 'A';
            ack[1] = seq;
            ack[2] = checksum(ack, 2);

            if (maybe_drop(loss_prob)) {
                printf("[RECV] Simulated drop of ACK for seq=%u\n", seq);
            } else {
                ssize_t s = sendto(sockfd, ack, 3, 0, (struct sockaddr *)&cliaddr, len);
                if (s < 0) perror("sendto");
                else printf("[RECV] Sent ACK for seq=%u\n", seq);
            }

            last_ack_seq = seq;
            expected_seq ^= 1; // expect next seq
        } else {
            // duplicate frame (already received) -> resend last ACK
            printf("[RECV] Received DUPLICATE frame seq=%u (expected %u). Resending ACK %u\n",
                   seq, expected_seq, last_ack_seq);
            unsigned char ack[3];
            ack[0] = 'A';
            ack[1] = last_ack_seq;
            ack[2] = checksum(ack, 2);
            if (maybe_drop(loss_prob)) {
                printf("[RECV] Simulated drop of duplicate ACK for seq=%u\n", last_ack_seq);
            } else {
                ssize_t s = sendto(sockfd, ack, 3, 0, (struct sockaddr *)&cliaddr, len);
                if (s < 0) perror("sendto");
            }
        }
    }

    close(sockfd);
    return 0;
}
