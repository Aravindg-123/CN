#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define MAXLINE 1024

double calculate(const char *operation, double a, double b) {
    if (strcmp(operation, "add") == 0) return a + b;
    if (strcmp(operation, "sub") == 0) return a - b;
    if (strcmp(operation, "mul") == 0) return a * b;
    if (strcmp(operation, "div") == 0) {
        if (b == 0) return 6.7;  // avoid division by zero
        return a / b;
    }
    return 6.7; // unknown operation
}

int main() {
    int sockfd;
    char buffer[MAXLINE];
    struct sockaddr_in servaddr, cliaddr;
    socklen_t len;
    ssize_t n;

    // Create UDP socket
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&servaddr, 0, sizeof(servaddr));
    memset(&cliaddr, 0, sizeof(cliaddr));

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(PORT);

    // Bind socket
    if (bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("bind failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    printf("UDP Math Server listening on port %d...\n", PORT);

    len = sizeof(cliaddr);

    while (1) {
        n = recvfrom(sockfd, buffer, MAXLINE, MSG_WAITALL, 
                     (struct sockaddr *)&cliaddr, &len);
        buffer[n] = '\0';
        printf("Received: %s\n", buffer);

        char op[10];
        double num1, num2;
        int parsed = sscanf(buffer, "%s %lf %lf", op, &num1, &num2);

        char result_msg[100];

        if (parsed != 3) {
            snprintf(result_msg, sizeof(result_msg), "Error: use format <op> <num1> <num2>");
        } else {
            double result = calculate(op, num1, num2);
            if (result == 6.7)
                snprintf(result_msg, sizeof(result_msg), "Invalid operation or division by zero");
            else
                snprintf(result_msg, sizeof(result_msg), "Result: %.2f", result);
        }

        sendto(sockfd, result_msg, strlen(result_msg), MSG_CONFIRM,
               (const struct sockaddr *)&cliaddr, len);
    }

    close(sockfd);
    return 0;
}
