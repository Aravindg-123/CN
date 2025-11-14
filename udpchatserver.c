#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#define PORT 8080
#define MAXLINE 1024

struct sockaddr_in cliaddr;
socklen_t len = sizeof(cliaddr);
int sockfd;

void *receive_messages(void *arg) {
    char buffer[MAXLINE];
    while(1) {
        memset(buffer, 0, sizeof(buffer));
        int n = recvfrom(sockfd, buffer, sizeof(buffer)-1, 0,
                         (struct sockaddr*)&cliaddr, &len);
        if(n > 0) {
            buffer[n] = '\0';
            printf("Client: %s\n", buffer);
        }
    }
    return NULL;
}

int main() {
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if(sockfd < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(PORT);

    if(bind(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    printf("UDP server listening on port %d...\n", PORT);

    pthread_t tid;
    pthread_create(&tid, NULL, receive_messages, NULL);

    char buffer[MAXLINE];
    while(1) {
        memset(buffer, 0, sizeof(buffer));
        printf("Server: ");
        fgets(buffer, sizeof(buffer), stdin);
        buffer[strcspn(buffer, "\n")] = 0;

        sendto(sockfd, buffer, strlen(buffer), 0,
               (struct sockaddr*)&cliaddr, len);
    }

    close(sockfd);
    return 0;
}
