#include "packet_builders.h"

#include <string.h>
#include <assert.h>

static void put_u16_be(uint8_t *dst, uint16_t v) {
    dst[0] = (uint8_t)(v >> 8);
    dst[1] = (uint8_t)(v & 0xFFu);
}

static uint32_t checksum_add16(uint32_t sum, const uint8_t *data, size_t len) {
    while (len >= 2) {
        sum += (uint32_t)((data[0] << 8) | data[1]);
        data += 2;
        len -= 2;
    }
    if (len == 1) {
        sum += (uint32_t)(data[0] << 8);
    }
    return sum;
}

static uint16_t checksum_finalize(uint32_t sum) {
    while (sum >> 16) {
        sum = (sum & 0xFFFFu) + (sum >> 16);
    }
    sum = ~sum;
    return (sum == 0) ? 0xFFFFu : (uint16_t)sum;
}

static uint16_t udp6_checksum(const uint8_t src[16], const uint8_t dst[16], const uint8_t *udp, size_t udp_len) {
    uint32_t sum = 0;
    sum = checksum_add16(sum, src, 16);
    sum = checksum_add16(sum, dst, 16);

    uint8_t len_bytes[4] = {
            (uint8_t)(udp_len >> 24),
            (uint8_t)(udp_len >> 16),
            (uint8_t)(udp_len >> 8),
            (uint8_t)(udp_len)
    };
    sum = checksum_add16(sum, len_bytes, sizeof(len_bytes));

    uint8_t nh[4] = {0, 0, 0, 17};
    sum = checksum_add16(sum, nh, sizeof(nh));

    sum = checksum_add16(sum, udp, udp_len);
    return checksum_finalize(sum);
}

size_t build_ipv6_udp_packet(uint8_t *buf,
                                    const uint8_t src_ip[16],
                                    const uint8_t dst_ip[16],
                                    uint16_t src_port,
                                    uint16_t dst_port,
                                    const uint8_t* payload,
                                    size_t length) {
    const size_t payload_len = length;
    const uint16_t udp_len = (uint16_t)(8u + payload_len);
    const uint16_t ipv6_payload_len = udp_len;
    const size_t total_len = 40u + udp_len;

    memset(buf, 0, total_len);
    buf[0] = 0x60;
    put_u16_be(buf + 4, ipv6_payload_len);
    buf[6] = 17;
    buf[7] = 0xff;
    memcpy(buf + 8, src_ip, 16);
    memcpy(buf + 24, dst_ip, 16);

    put_u16_be(buf + 40, src_port);
    put_u16_be(buf + 42, dst_port);
    put_u16_be(buf + 44, udp_len);
    memcpy(buf + 48, payload, payload_len);

    buf[46] = 0;
    buf[47] = 0;
    uint16_t csum = udp6_checksum(src_ip, dst_ip, buf + 40, udp_len);
    put_u16_be(buf + 46, csum);
    return total_len;
}

static size_t append_coap_option(uint8_t *buf, size_t off, uint8_t delta, const char *segment) {
    size_t len = strlen(segment);
    assert(delta < 16);
    assert(len < 16);
    buf[off++] = (uint8_t)((delta << 4) | (uint8_t)len);
    memcpy(buf + off, segment, len);
    return off + len;
}

size_t build_coap_message(uint8_t *buf,
                                 uint8_t type,
                                 uint8_t code,
                                 const char *const paths[],
                                 size_t path_count,
                                 const uint8_t *payload,
                                 size_t payload_len) {
    size_t off = 0;
    const uint16_t msg_id = 0x1234u;
    buf[off++] = (uint8_t)((0x01u << 6) | ((type & 0x03u) << 4) | 0x00u);
    buf[off++] = code;
    buf[off++] = (uint8_t)(msg_id >> 8);
    buf[off++] = (uint8_t)(msg_id & 0xFFu);

    if (path_count > 0) {
        assert(paths != NULL);
        for (size_t i = 0; i < path_count; ++i) {
            off = append_coap_option(buf, off, (i == 0) ? 11u : 0u, paths[i]);
        }
    }

    if (payload_len > 0) {
        assert(payload != NULL);
        buf[off++] = 0xFFu;
        memcpy(buf + off, payload, payload_len);
        off += payload_len;
    }

    return off;
}