#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#define PORT 8080
#define MAXLINE 1024

int sockfd;

void *receive_messages(void *arg) {
    char buffer[MAXLINE];
    while(1) {
        memset(buffer, 0, sizeof(buffer));
        int n = read(sockfd, buffer, sizeof(buffer)-1);
        if(n <= 0) {
            printf("Server disconnected.\n");
            exit(0);
        }
        printf("Server: %s\n", buffer);
    }
    return NULL;
}

int main() {
    struct sockaddr_in servaddr;
    char server_ip[50];

    printf("Enter server IP: ");
    scanf("%s", server_ip);
    getchar(); // consume newline

    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    inet_pton(AF_INET, server_ip, &servaddr.sin_addr);

    if(connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0) {
        perror("Connection failed");
        exit(EXIT_FAILURE);
    }

    printf("Connected to server!\n");

    pthread_t tid;
    pthread_create(&tid, NULL, receive_messages, NULL);

    char buffer[MAXLINE];
    while(1) {
        memset(buffer, 0, sizeof(buffer));
        printf("Client: ");
        fgets(buffer, sizeof(buffer), stdin);
        buffer[strcspn(buffer, "\n")] = 0;
        write(sockfd, buffer, strlen(buffer));
    }

    close(sockfd);
    return 0;
}
