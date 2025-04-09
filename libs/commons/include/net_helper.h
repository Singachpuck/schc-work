#ifndef NET_HELPER_H
#define NET_HELPER_H

#include <stddef.h>
#include <stdint.h>

#define IPV4_ADDR_BYTES 4
// With \0 terminator
#define IPV4_STR_MAX_LEN 16

#define IPV6_HEADERS_BYTES 40
#define IPV6_ADDR_BYTES 16
#define IPV6_ADDR_PREFIX_BYTES 8

#define UDP_HEADERS_BYTES 8
#define UDP_PORT_BYTES 2
#define UDP_PORT_STR_MAX_LEN 6


int generate_ipv6_udp_packet(uint8_t* packet,
        const char* src_addr, const char* src_port,
        const char* dst_addr, const char* dst_port,
        const uint8_t* payload, const size_t payload_size);

#endif