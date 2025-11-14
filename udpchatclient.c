/*gcc udp_chat_server.c -o udp_server -lpthread
gcc udp_chat_client.c -o udp_client -lpthread
./udp_server*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#define PORT 8080
#define MAXLINE 1024

struct sockaddr_in servaddr;
int sockfd;

void *receive_messages(void *arg) {
    char buffer[MAXLINE];
    socklen_t len = sizeof(servaddr);
    while(1) {
        memset(buffer, 0, sizeof(buffer));
        int n = recvfrom(sockfd, buffer, sizeof(buffer)-1, 0,
                         (struct sockaddr*)&servaddr, &len);
        if(n > 0) {
            buffer[n] = '\0';
            printf("Server: %s\n", buffer);
        }
    }
    return NULL;
}

int main() {
    char server_ip[50];
    printf("Enter server IP: ");
    scanf("%s", server_ip);
    getchar(); // consume newline

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if(sockfd < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    inet_pton(AF_INET, server_ip, &servaddr.sin_addr);

    pthread_t tid;
    pthread_create(&tid, NULL, receive_messages, NULL);

    char buffer[MAXLINE];
    socklen_t len = sizeof(servaddr);
    while(1) {
        memset(buffer, 0, sizeof(buffer));
        printf("Client: ");
        fgets(buffer, sizeof(buffer), stdin);
        buffer[strcspn(buffer, "\n")] = 0;

        sendto(sockfd, buffer, strlen(buffer), 0,
               (struct sockaddr*)&servaddr, len);
    }

    close(sockfd);
    return 0;
}
