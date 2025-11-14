#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#define PORT 8080
#define MAXLINE 1024

int connfd; // global connection descriptor

void *receive_messages(void *arg) {
    char buffer[MAXLINE];
    while(1) {
        memset(buffer, 0, sizeof(buffer));
        int n = read(connfd, buffer, sizeof(buffer)-1);
        if(n <= 0) {
            printf("Client disconnected.\n");
            exit(0);
        }
        printf("Client: %s\n", buffer);
    }
    return NULL;
}

int main() {
    int sockfd;
    struct sockaddr_in servaddr, cliaddr;
    socklen_t len = sizeof(cliaddr);

    // Create TCP socket
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(PORT);

    // Bind
    if(bind(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    // Listen
    if(listen(sockfd, 1) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d...\n", PORT);

    // Accept connection
    connfd = accept(sockfd, (struct sockaddr*)&cliaddr, &len);
    if(connfd < 0) {
        perror("Accept failed");
        exit(EXIT_FAILURE);
    }

    printf("Client connected!\n");

    // Create thread to receive messages
    pthread_t tid;
    pthread_create(&tid, NULL, receive_messages, NULL);

    // Main thread: send messages
    char buffer[MAXLINE];
    while(1) {
        memset(buffer, 0, sizeof(buffer));
        printf("Server: ");
        fgets(buffer, sizeof(buffer), stdin);
        buffer[strcspn(buffer, "\n")] = 0; // remove newline
        write(connfd, buffer, strlen(buffer));
    }

    close(sockfd);
    return 0;
}
