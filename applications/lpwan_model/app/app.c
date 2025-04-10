#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <coap3/coap.h>


#include "common_defs.h"

#define RECEIVE_BUFFER_SIZE 1500
#define COAP_PDU_MAX_SIZE 1024

uint8_t buffer[RECEIVE_BUFFER_SIZE];

// TODO: Create App to receive an IP packet and process it --> add CoAP support
int main() {
    int sockfd;
    struct sockaddr_in6 server_addr = {0},
                        client_addr = {0};

    socklen_t addr_len = sizeof(server_addr);
    const uint16_t port = atoi(APP_PORT);

    if ((sockfd = socket(AF_INET6, SOCK_RAW, IPPROTO_UDP)) < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    server_addr.sin6_family = AF_INET6;
    server_addr.sin6_addr = in6addr_any;
    server_addr.sin6_port = htons(port);

    if (bind(sockfd, (struct sockaddr*)&server_addr, addr_len) < 0) {
        perror("bind failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    coap_set_log_level(COAP_LOG_INFO);
    coap_pdu_t *pdu = coap_pdu_init(0, 0, 0, COAP_PDU_MAX_SIZE);
    if (!pdu) {
        perror("Failed to allocate CoAP PDU\n");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    printf("CoAP server is listening on port %s...\n", APP_PORT);

    while (1) {
        ssize_t recv_len = recvfrom(sockfd, buffer, RECEIVE_BUFFER_SIZE, 0,
                                    (struct sockaddr*)&client_addr, &addr_len);
        if (recv_len < 0) {
            perror("recvfrom failed");
            continue;
        }

        if (recv_len < 16) {
            perror("packet is corrupt");
            continue;
        }

        if (ntohs(*(uint16_t*) (buffer + 2)) == port) {
            // TODO: Add logging util to the commons lib
            for (uint16_t i = 0; i < recv_len; i++) {
                const uint8_t b = buffer[i];
                printf("%02X ", b);
                if ((i + 1) % 10 == 0)
                    printf("\n");
            }
            printf("\n");

            coap_pdu_parse(COAP_PROTO_UDP, buffer + 8, recv_len - 8, pdu);

            coap_show_pdu(LOG_INFO, pdu);

            fflush(stdout);
            fflush(stderr);
        }
    }

    coap_delete_pdu(pdu);
    close(sockfd);
    return 0;
}