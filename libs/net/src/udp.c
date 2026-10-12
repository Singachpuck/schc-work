#include <stddef.h>
#include <netinet/ip6.h>
#include <netinet/udp.h>

#include "net/udp.h"

bool is_ipv6_udp_packet(const uint8_t *packet, size_t packet_size) {
  if (packet == NULL || packet_size < sizeof(struct ip6_hdr) + sizeof(struct udphdr)) {
    return false;
  }

  if ((packet[0] >> 4) != IPv6_VERSION) {
    return false;
  }

  const struct ip6_hdr *ip6 = (const struct ip6_hdr *) packet;
  return ip6->ip6_nxt == IPPROTO_UDP;
}

/* Add `n` bytes to a running ones'-complement sum, treating data as
 * big-endian 16-bit words. Odd trailing byte is padded with a zero byte. */
static uint64_t csum_add(uint64_t sum, const uint8_t *p, size_t n)
{
    while (n > 1) {
        sum += ((uint32_t)p[0] << 8) | p[1];
        p += 2;
        n -= 2;
    }
    if (n)
        sum += (uint32_t)p[0] << 8;
    return sum;
}

/* Returns the checksum in network byte order (store directly in uh_sum).
 * `packet` points at the UDP header; the uh_sum field must be zero. */
uint16_t udp_checksum(const uint8_t *packet, size_t size,
                      struct in6_addr *src_addr,
                      struct in6_addr *dst_addr)
{
    uint64_t sum = 0;

    /* IPv6 pseudo-header (RFC 8200 §8.1) */
    sum = csum_add(sum, src_addr->s6_addr, 16);
    sum = csum_add(sum, dst_addr->s6_addr, 16);
    sum += (uint32_t)(size >> 16) & 0xFFFF;   /* upper-layer length, high half */
    sum += (uint32_t)size & 0xFFFF;           /* upper-layer length, low half  */
    sum += IPPROTO_UDP;                       /* next header (3 zero bytes + 17) */

    /* UDP header + payload */
    sum = csum_add(sum, packet, size);

    /* Fold carries */
    while (sum >> 16)
        sum = (sum & 0xFFFF) + (sum >> 16);

    uint16_t result = (uint16_t)~sum;
    if (result == 0)
        result = 0xFFFF;          /* 0 is reserved for "no checksum" in UDP */

    return htons(result);
}

void update_udp_checksum(uint8_t *packet)
{
    struct ip6_hdr *iphdr = (struct ip6_hdr *)packet;
    struct udphdr *uhdr = (struct udphdr *)(packet + sizeof(struct ip6_hdr));
    size_t ulen = ntohs(uhdr->uh_ulen);

    uhdr->uh_sum = 0;
    uhdr->uh_sum = udp_checksum((const uint8_t *)uhdr, ulen,
                                &iphdr->ip6_src, &iphdr->ip6_dst);
}

void update_ip6_udp_len(uint8_t *packet, size_t size)
{
  struct ip6_hdr *iphdr = (struct ip6_hdr *)packet;
  iphdr->ip6_plen = htons(sizeof(struct udphdr) + size);
  struct udphdr *uhdr = (struct udphdr *)(packet + sizeof(struct ip6_hdr));
  uhdr->uh_ulen = htons(sizeof(struct udphdr) + size);
}

uint16_t ipv6_udp_payload_len(const uint8_t *packet) {
    const struct udphdr *uhdr = (const struct udphdr *)((const char *)packet + sizeof(struct ip6_hdr));
    uint16_t udp_total_len = ntohs(uhdr->uh_ulen);

    // Ensure the UDP total length is at least the size of the UDP header
    if (udp_total_len < sizeof(struct udphdr)) {
        return 0;
    }

    return udp_total_len - sizeof(struct udphdr);
}

// CoAP version is 1, which is represented by 01 in the two most significant bits of the first byte.
#define COAP_VERSION_MASK 0xC0 // 1100 0000
#define COAP_VERSION_1    0x40 // 0100 0000

bool is_coap_packet(const uint8_t* raw_coap, size_t pkt_len) {
  if (pkt_len < 1) {
    return false;
  }
  return (raw_coap[0] & COAP_VERSION_MASK) == COAP_VERSION_1;
}