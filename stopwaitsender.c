/*
 stopwait_sender.c
 Usage: ./stopwait_sender <receiver-ip> <receiver-port> <loss_prob> <timeout_ms>
 Example: ./stopwait_sender 127.0.0.1 9000 0.2 2000
 loss_prob = probability [0.0..1.0] that an outgoing packet is dropped (for testing)
 timeout_ms = retransmission timeout in milliseconds
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <stdint.h>

#define MAXDATA 512
#define BUF_SIZE (1 + 2 + 1 + MAXDATA) // seq + len + chksum + data

// simple 8-bit checksum: sum of bytes modulo 256
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
    if (argc < 5) {
        fprintf(stderr, "Usage: %s <recv-ip> <recv-port> <loss_prob> <timeout_ms>\n", argv[0]);
        return 1;
    }

    const char *recv_ip = argv[1];
    int recv_port = atoi(argv[2]);
    double loss_prob = atof(argv[3]);
    int timeout_ms = atoi(argv[4]);

    srand((unsigned)time(NULL));

    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) { perror("socket"); return 1; }

    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(recv_port);
    if (inet_pton(AF_INET, recv_ip, &servaddr.sin_addr) <= 0) {
        fprintf(stderr, "Invalid address\n"); close(sockfd); return 1;
    }

    unsigned char seq = 0;
    char line[1024];

    printf("Stop-and-Wait Sender started. Type messages to send. Ctrl+D to exit.\n");
    while (1) {
        printf("Enter message: ");
        if (!fgets(line, sizeof(line), stdin)) break;
        size_t msglen = strlen(line);
        // remove newline if present
        if (msglen > 0 && line[msglen-1] == '\n') { line[msglen-1] = '\0'; --msglen; }

        // build frame
        unsigned char buffer[BUF_SIZE];
        memset(buffer, 0, sizeof(buffer));
        buffer[0] = seq;
        uint16_t nlen = (uint16_t)msglen;
        uint16_t nlen_net = htons(nlen);
        memcpy(&buffer[1], &nlen_net, 2);
        memcpy(&buffer[1+2+1], line, msglen); // leave chksum for buffer[3]
        // compute checksum over seq + len + data (i.e., first 3..)
        int chksum_len = 1 + 2 + (int)msglen;
        unsigned char ch = checksum(buffer, chksum_len);
        buffer[1+2] = ch; // place checksum at buffer[3]

        int sent = 0;
        while (!sent) {
            // simulate drop
            if (maybe_drop(loss_prob)) {
                printf("[SENDER] Simulated drop of frame seq=%u\n", seq);
            } else {
                ssize_t s = sendto(sockfd, buffer, 1+2+1+msglen, 0,
                                   (struct sockaddr *)&servaddr, sizeof(servaddr));
                if (s < 0) perror("sendto");
                else printf("[SENDER] Sent frame seq=%u (%zd bytes)\n", seq, s);
            }

            // wait for ACK with timeout
            fd_set readfds;
            FD_ZERO(&readfds);
            FD_SET(sockfd, &readfds);
            struct timeval tv;
            tv.tv_sec = timeout_ms / 1000;
            tv.tv_usec = (timeout_ms % 1000) * 1000;

            int rv = select(sockfd + 1, &readfds, NULL, NULL, &tv);
            if (rv < 0) { perror("select"); close(sockfd); return 1; }
            else if (rv == 0) {
                // timeout -> retransmit
                printf("[SENDER] Timeout waiting for ACK for seq=%u, retransmitting...\n", seq);
                continue;
            } else {
                unsigned char ackbuf[16];
                struct sockaddr_in from;
                socklen_t fromlen = sizeof(from);
                ssize_t r = recvfrom(sockfd, ackbuf, sizeof(ackbuf), 0,
                                     (struct sockaddr *)&from, &fromlen);
                if (r <= 0) { perror("recvfrom"); continue; }

                // validate ACK: expected format: 'A', ack_seq, chksum
                if (r < 3) { printf("[SENDER] Ignoring malformed ACK (len %zd)\n", r); continue; }
                unsigned char type = ackbuf[0];
                unsigned char ack_seq = ackbuf[1];
                unsigned char recv_ch = ackbuf[2];
                unsigned char calc = checksum(ackbuf, 2); // checksum on first two bytes
                if (type != 'A' || calc != recv_ch) {
                    printf("[SENDER] Received invalid ACK or checksum mismatch, ignoring\n");
                    continue;
                }
                if (ack_seq == seq) {
                    printf("[SENDER] Received ACK for seq=%u\n", ack_seq);
                    seq ^= 1; // next sequence number
                    sent = 1;
                } else {
                    printf("[SENDER] Received ACK for seq=%u (expected %u) -> ignoring\n", ack_seq, seq);
                }
            }
        } // end sending loop for this frame
    } // end read loop

    close(sockfd);
    printf("Sender exiting.\n");
    return 0;
}
