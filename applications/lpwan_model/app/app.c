#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include "common_defs.h"

#define RECEIVE_BUFFER_SIZE 1500

// TODO: Create App to receive an IP packet and process it --> add CoAP support
int main() {
    int sockfd;
    struct sockaddr_in6 server_addr, client_addr;
    char buffer[RECEIVE_BUFFER_SIZE];
    socklen_t addr_len = sizeof(client_addr);
    const uint16_t port = atoi(APP_PORT);

    if ((sockfd = socket(AF_INET6, SOCK_RAW, IPPROTO_UDP)) < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin6_family = AF_INET6;
    server_addr.sin6_addr = in6addr_any;
    server_addr.sin6_port = htons(port);

    if (bind(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    printf("UDP server is listening on port %s...\n", APP_PORT);

    while (1) {
        ssize_t recv_len = recvfrom(sockfd, buffer, RECEIVE_BUFFER_SIZE, 0,
                                    (struct sockaddr*)&client_addr, &addr_len);
        if (recv_len < 0) {
            perror("recvfrom failed");
            close(sockfd);
            exit(EXIT_FAILURE);
        }

        if (ntohs(*(uint16_t*) (buffer + 2)) == port) {
            uint8_t b;
            for (uint16_t i = 0; i < recv_len; i++) {
                b = buffer[i];
                printf("%02X ", b);
                if ((i + 1) % 10 == 0)
                    printf("\n");
            }
            printf("\n");
        }
    }

    close(sockfd);
    return 0;
}