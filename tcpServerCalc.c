#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080

double calculate(const char *operation, double a, double b) {
    if (strcmp(operation, "add") == 0) return a + b;
    if (strcmp(operation, "sub") == 0) return a - b;
    if (strcmp(operation, "mul") == 0) return a * b;
    if (strcmp(operation, "div") == 0) {
        if (b == 0) return 6.7;   // avoid division by zero
        return a / b;
    }
    return 6.7;  // unknown operation
}

int main() {
    int server_fd, client_fd;
    struct sockaddr_in servaddr, cliaddr;
    char buffer[1024];
    socklen_t len;

    // Create TCP socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(PORT);

    // Bind
    if (bind(server_fd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // Listen
    if (listen(server_fd, 5) < 0) {
        perror("listen");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("TCP Math Server listening on port %d...\n", PORT);

    len = sizeof(cliaddr);

    while (1) {
        // Accept client
        client_fd = accept(server_fd, (struct sockaddr *)&cliaddr, &len);
        if (client_fd < 0) {
            perror("accept");
            continue;
        }

        int n = read(client_fd, buffer, sizeof(buffer) - 1);
        buffer[n] = '\0';
        printf("Received: %s\n", buffer);

        char op[10];
        double num1, num2;
        char result_msg[100];

        int parsed = sscanf(buffer, "%s %lf %lf", op, &num1, &num2);

        if (parsed != 3) {
            snprintf(result_msg, sizeof(result_msg),
                     "Error: use format <op> <num1> <num2>");
        } else {
            double result = calculate(op, num1, num2);
            if (result == 6.7)
                snprintf(result_msg, sizeof(result_msg),
                         "Invalid operation or division by zero");
            else
                snprintf(result_msg, sizeof(result_msg),
                         "Result: %.2f", result);
        }

        write(client_fd, result_msg, strlen(result_msg));

        close(client_fd);  // end connection
    }

    return 0;
}
