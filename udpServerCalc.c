#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define MAXLINE 1024

int main() {
    int sockfd;
    char buffer[MAXLINE];
    struct sockaddr_in servaddr;
    socklen_t len;

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    servaddr.sin_addr.s_addr = INADDR_ANY; // for localhost

    len = sizeof(servaddr);

    printf("UDP Math Client started. Format: <op> <num1> <num2>\n");
    printf("Operations: add, sub, mul, div\n");
    printf("Example: add 10 20\n\n");

    while (1) {
        char msg[MAXLINE];
        printf("Enter operation: ");
        fgets(msg, MAXLINE, stdin);

        // Remove newline
        msg[strcspn(msg, "\n")] = 0;

        if (strcmp(msg, "exit") == 0) {
            printf("Exiting client.\n");
            break;
        }

        sendto(sockfd, msg, strlen(msg), MSG_CONFIRM,
               (const struct sockaddr *)&servaddr, len);

        int n = recvfrom(sockfd, buffer, MAXLINE, MSG_WAITALL,
                         (struct sockaddr *)&servaddr, &len);
        buffer[n] = '\0';
        printf("Server: %s\n\n", buffer);
    }
    close(sockfd);
    return 0;
}
