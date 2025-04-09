#include <stdlib.h>

#include <fullsdkfragapi.h>
#include <fullsdkmgt.h>
#include <fullsdknet.h>
#include <platform.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netinet/in.h>
// #include <sys/socket.h>

#include "common_defs.h"
#include "schc_helper.h"

static sdk_mode_t sdk_mode = SDK_APP_MODE;

static bool app_process_request = false;
static bool app_send_data_request = false;

void receive() {
  int sockfd;
  struct sockaddr_in6 server_addr, client_addr;
  char buffer[1024];
  socklen_t addr_len = sizeof(client_addr);

  // Create IPv6 UDP socket
  if ((sockfd = socket(AF_INET6, SOCK_DGRAM, 0)) < 0) {
    perror("Socket creation failed");
    return;
  }

  // Configure server address structure for IPv6
  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin6_family = AF_INET6;
  server_addr.sin6_addr = in6addr_any;  // Listen on any IPv6 address
  server_addr.sin6_port = htons(atoi(DEVICE_PORT));

  // Set the socket receive timeout
  struct timeval timeout;
  timeout.tv_sec = 3;
  timeout.tv_usec = 0;

  if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
    perror("setsockopt failed");
    close(sockfd);
    return;
  }

  // Bind the socket to the address and port
  if (bind(sockfd, (const struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
    perror("Bind failed");
    close(sockfd);
    return;
  }

  printf("Listening for IPv6 UDP packets on port %s...\n", DEVICE_PORT);

  // Receive data from client
  ssize_t recv_len = recvfrom(sockfd, buffer, 1024, 0, (struct sockaddr *)&client_addr, &addr_len);
  if (recv_len < 0) {
    perror("Receive failed");
    close(sockfd);
    return;
  }

  char client_ip[INET6_ADDRSTRLEN];
  inet_ntop(AF_INET6, &client_addr.sin6_addr, client_ip, sizeof(client_ip));
  printf("Received message from [%s]\n", client_ip);

  close(sockfd);
}

// NET callbacks.
static void net_transmission_result(net_status_t status, uint16_t error) {
  if (status != NET_SUCCESS) {
    PRINT_MSG("transmission result KO (status %d)\n", status);
    return;
  }
  PRINT_MSG("transmission result OK\n");
}

static void net_data_received(const uint8_t *buffer, uint16_t data_size,
                              net_status_t status) {
  PRINT_MSG("data received %d bytes - status %d\n", data_size, status);

  if (status != NET_SUCCESS) {
    return;
  }

  uint8_t b;
  for (uint16_t i = 0; i < data_size; i++) {
    b = buffer[i];
    PRINT_MSG("%02X ", b);
    if ((i + 1) % 10 == 0)
      PRINT_MSG("\n");
  }
  PRINT_MSG("\n");

  app_process_request = true;
  app_send_data_request = true;

  int sockfd = socket(AF_INET6, SOCK_RAW, IPPROTO_RAW);
  if (sockfd < 0) {
    perror("socket() failed");
    return;
  }

  struct sockaddr_in6 dest_addr = {0};
  dest_addr.sin6_family = AF_INET6;
  if (inet_pton(AF_INET6, APP_ADDR, &dest_addr.sin6_addr) != 1) {
    perror("inet_pton() failed");
    close(sockfd);
    return;
  }

  ssize_t sent_bytes = sendto(sockfd, buffer, data_size, 0,
                            (struct sockaddr *)&dest_addr, sizeof(dest_addr));

  if (sent_bytes == -1) {
    perror("sendto() failed");
    close(sockfd);
    return;
  }

  PRINT_MSG("Sent %ld bytes to %s:%s\n", sent_bytes, APP_ADDR, APP_PORT);

  close(sockfd);
}

static net_callbacks_t net_callbacks = {
  net_transmission_result,
  net_data_received
};

// TODO: Create template for CoAP
/**
 * Device sends a dummy CoAP packet with sensor value to the SCHC Core using IPv4 tunnel.
 */
int main() {
  PRINT_MSG("FullSDK version: %s\n", mgt_get_version());

  start_config();

  with_mtu(100);
  with_sdk_mode(sdk_mode);
  with_tunnel(
  CORE_TUNNEL_ADDR,
  CORE_TUNNEL_PORT,
  DEVICE_TUNNEL_ADDR,
  DEVICE_TUNNEL_PORT
  );

  // I don't know why, should be reverted at the receiver's side
  with_ipv6_src_param(APP_ADDR);
  with_ipv6_udp_src_param(APP_PORT);
  with_ipv6_dst_param(DEVICE_ADDR);
  with_ipv6_udp_dst_param(DEVICE_PORT);
  with_net_callbacks(net_callbacks);

  if (config_schc() != SCHC_CONFIG_OK) {
    return EXIT_FAILURE;
  }

  while (true) {
    if (mgt_process_request) {
      mgt_process_request = false;
      const mgt_status_t mgt_status = mgt_process();

      if (mgt_status != MGT_SUCCESS) {
        PRINT_MSG("Error processing SCHC packet (%d)", mgt_status);
        return EXIT_FAILURE;
      }
    }

    if (!mgt_process_request)
    {
      platform_enter_low_power_ll();
    }
  }

  return EXIT_SUCCESS;
}
