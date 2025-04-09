
#include "net_helper.h"

#include <string.h>
#include <arpa/inet.h>
#include <netinet/ip6.h>
#include <netinet/udp.h>

#include <stdio.h>
#include <stdlib.h>

// UDP checksum calculation function
uint16_t checksum(void *buffer, int length) {
    uint16_t *data = buffer;
    unsigned long sum = 0;

    while (length > 1) {
        sum += *data++;
        length -= 2;
    }

    if (length > 0) {
        sum += *(unsigned char *)data;
    }

    while (sum >> 16) {
        sum = (sum & 0xFFFF) + (sum >> 16);
    }

    return ~sum;
}

// Function to compute UDP checksum with pseudo-header for IPv6
unsigned short udp_checksum(struct in6_addr *src, struct in6_addr *dst,
                            struct udphdr *udp, const uint8_t *payload, const size_t payload_len) {
    unsigned char pseudo_header[40 + sizeof(struct udphdr) + payload_len];
    memset(pseudo_header, 0, sizeof(pseudo_header));

    // Copy IPv6 addresses (pseudo-header)
    memcpy(pseudo_header, src, 16);
    memcpy(pseudo_header + 16, dst, 16);

    // Add UDP Length
    unsigned int udp_len = htonl(sizeof(struct udphdr) + payload_len);
    memcpy(pseudo_header + 32, &udp_len, 4);

    // Set Next Header (UDP = 17)
    pseudo_header[39] = IPPROTO_UDP;

    // Copy UDP Header
    memcpy(pseudo_header + 40, udp, sizeof(struct udphdr));

    // Copy Payload
    memcpy(pseudo_header + 40 + sizeof(struct udphdr), payload, payload_len);

    // Compute checksum
    return checksum(pseudo_header, sizeof(pseudo_header));
}

int generate_ipv6_udp_packet(uint8_t* packet,
        const char* src_addr, const char* src_port,
        const char* dst_addr, const char* dst_port,
        const uint8_t* payload, const size_t payload_size) {

    struct ip6_hdr *ip6h;
    struct udphdr *udph;

    memset(packet, 0, IPV6_HEADERS_BYTES + UDP_HEADERS_BYTES + payload_size);

    // IPv6 header
    ip6h = (struct ip6_hdr *) packet;
    ip6h->ip6_vfc = (6 << 4); // IPv6 version
    ip6h->ip6_flow |= htonl(0x6724d);
    ip6h->ip6_plen = htons(UDP_HEADERS_BYTES + payload_size); // Payload length
    ip6h->ip6_nxt = IPPROTO_UDP;
    ip6h->ip6_hlim = 64; // Hop limit
    inet_pton(AF_INET6, src_addr, &(ip6h->ip6_src)); // Source IP
    inet_pton(AF_INET6, dst_addr, &(ip6h->ip6_dst)); // Destination IP

    // UDP header
    udph = (struct udphdr *)(packet + IPV6_HEADERS_BYTES);
    udph->uh_sport = htons(atoi(src_port));
    udph->uh_dport = htons(atoi(dst_port));
    udph->uh_ulen = htons(UDP_HEADERS_BYTES + payload_size);
    // udph->uh_sum = udp_checksum(&(ip6h->ip6_src), &(ip6h->ip6_dst), udph, payload, payload_size);
    // TODO: Verify checksum
    udph->uh_sum = htons(0x0039);

    // Payload
    memcpy(packet + IPV6_HEADERS_BYTES + UDP_HEADERS_BYTES, payload, payload_size);

    return 0;
}
