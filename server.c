#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/select.h>

#define PORT 12345
#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024

int main() {
    int server_fd, client_fd, max_fd, activity;
    int clients[MAX_CLIENTS] = {0};
    struct sockaddr_in server_addr, client_addr;
    socklen_t addrlen = sizeof(client_addr);
    char buffer[BUFFER_SIZE];
    fd_set read_fds;
 
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("socket");
        exit(1);
    }

     
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    
    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("bind");
        close(server_fd);
        exit(1);
    }

   
    if (listen(server_fd, 5) == -1) {
        perror("listen");
        close(server_fd);
        exit(1);
    }

    printf("Server is running on port %d.\n", PORT);

    while (1) {
        FD_ZERO(&read_fds);
        FD_SET(server_fd, &read_fds);
        max_fd = server_fd;

         
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (clients[i] > 0) {
                FD_SET(clients[i], &read_fds);
                if (clients[i] > max_fd)
                    max_fd = clients[i];
            }
        }

        // select 
        activity = select(max_fd + 1, &read_fds, NULL, NULL, NULL);
        if (activity < 0) {
            perror("select");
            exit(1);
        }

         
        if (FD_ISSET(server_fd, &read_fds)) {
            client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &addrlen);
            if (client_fd < 0) {
                perror("accept");
                continue;
            }

            printf("Client Connection: %d\n", client_fd);

            for (int i = 0; i < MAX_CLIENTS; i++) {
                if (clients[i] == 0) {
                    clients[i] = client_fd;
                    break;
                }
            }
        }

        
        for (int i = 0; i < MAX_CLIENTS; i++) {
            int sock = clients[i];
            if (sock > 0 && FD_ISSET(sock, &read_fds)) {
                int bytes = recv(sock, buffer, BUFFER_SIZE - 1, 0);
                if (bytes <= 0) {
                    printf("Client %d connection terminated\n", sock);
                    close(sock);
                    FD_CLR(sock, &read_fds);
                    clients[i] = 0;
                } else {
                    buffer[bytes] = '\0';

                     
                    if (strncmp(buffer, "/exit", 5) == 0) {
                        printf("Client %d exit\n", sock);
                        snprintf(buffer, sizeof(buffer), "Client %d exit.\n", sock);
                        for (int j = 0; j < MAX_CLIENTS; j++) {
                            if (clients[j] > 0 && clients[j] != sock)
                                send(clients[j], buffer, strlen(buffer), 0);
                        }
                        close(sock);
                        clients[i] = 0;
                        continue;
                    }

                     
                    for (int j = 0; j < MAX_CLIENTS; j++) {
                        if (clients[j] > 0 && clients[j] != sock)
                            send(clients[j], buffer, strlen(buffer), 0);
                    }
                }
            }
        }
    }

    close(server_fd);
    return 0;
}
