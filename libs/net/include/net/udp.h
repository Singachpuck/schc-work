#ifndef UDP_H
#define UDP_H

#include <stdint.h>
#include <stdbool.h>

#define IPv6_VERSION 6

bool is_ipv6_udp_packet(const uint8_t *packet, size_t packet_size);
uint16_t udp_checksum(const uint8_t *packet, size_t size,
             struct in6_addr *src_addr, struct in6_addr *dst_addr);
void update_udp_checksum(uint8_t *packet);
void update_ip6_udp_len(uint8_t *packet, size_t payload_size);
uint16_t ipv6_udp_payload_len(const uint8_t *packet);

#endif // UDP_H