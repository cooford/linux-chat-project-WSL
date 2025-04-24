#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/select.h>

#define SERVER_IP "127.0.0.1"
#define PORT 12345
#define BUFFER_SIZE 1024

int main() {
    int sock;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];
    fd_set read_fds;

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        perror("socket");
        exit(1);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr);

    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("connect");
        close(sock);
        exit(1);
    }

    printf("Connected to server. Type a message (/exit exit)\n");

    while (1) {
        FD_ZERO(&read_fds);
        FD_SET(0, &read_fds);      
        FD_SET(sock, &read_fds);   

        select(sock + 1, &read_fds, NULL, NULL, NULL);

        if (FD_ISSET(0, &read_fds)) {
            fgets(buffer, BUFFER_SIZE, stdin);
            buffer[strcspn(buffer, "\n")] = '\0';  

            send(sock, buffer, strlen(buffer), 0);

            if (strncmp(buffer, "/exit", 5) == 0) {
                printf("End the chat\n");
                break;
            }
        }

        if (FD_ISSET(sock, &read_fds)) {
            int bytes = recv(sock, buffer, BUFFER_SIZE - 1, 0);
            if (bytes <= 0) {
                printf("The connection to the server has been terminated.\n");
                break;
            }
            buffer[bytes] = '\0';
            printf("[chats] %s\n", buffer);//print
        }
    }

    close(sock);
    return 0;
}
