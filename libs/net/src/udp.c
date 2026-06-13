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

uint16_t udp_checksum(const uint8_t *packet, size_t size,
                      struct in6_addr *src_addr, struct in6_addr *dst_addr)
{
  const uint16_t *buf = (const uint16_t *)packet;
  uint16_t *ip_src = (uint16_t *)src_addr;
  uint16_t *ip_dst = (uint16_t *)dst_addr;
  uint32_t sum = 0;
  size_t len = size;

  while (len > 1)
  {
    sum += *buf++;
    if (sum & 0x80000000)
      sum = (sum & 0xFFFF) + (sum >> 16);
    len -= 2;
  }
  if (len & 1)
    sum += *((char *)buf);

  for (int i = 0; i < 8; i++)
    sum += ip_src[i];

  for (int i = 0; i < 8; i++)
    sum += ip_dst[i];

  sum += htons(IPPROTO_UDP);
  sum += htons(size);

  while (sum >> 16)
    sum = (sum & 0xFFFF) + (sum >> 16);

  return ((uint16_t)~sum);
}

void update_udp_checksum(uint8_t *packet)
{
  struct ip6_hdr *iphdr = (struct ip6_hdr *)packet;

  struct udphdr *uhdr = (struct udphdr *)(packet + sizeof(struct ip6_hdr));
  uhdr->uh_sum = 0;
  uhdr->uh_sum = udp_checksum((uint8_t*) uhdr, ntohs(uhdr->uh_ulen),
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