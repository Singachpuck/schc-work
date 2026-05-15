#pragma once

#include <stddef.h>
#include <stdint.h>

#define IPv6_NET_PREFIX_LEN 8
#define IPv6_IID_LEN 8
#define IPv6_ADDR_LEN 16
#define UDP_PORT_LEN 2

size_t build_ipv6_udp_packet(uint8_t *buf,
                             const uint8_t src_ip[16],
                             const uint8_t dst_ip[16],
                             uint16_t src_port,
                             uint16_t dst_port,
                             const uint8_t* payload,
                             size_t length);

size_t build_coap_message(uint8_t *buf,
                          uint8_t type,
                          uint8_t code,
                          const char *const paths[],
                          size_t path_count,
                          const uint8_t *payload,
                          size_t payload_len);