#include "schc_al.h"

#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <net/if.h>
#include <stdlib.h>

#include <fullsdknet.h>
#include <platform.h>

#include "net/tun.h"
#include "net/udp.h"
#include "net/net_helper.h"
#include "schc_al_params.h"
#include "oscore_proxy.h"

// TODO: Change to smaller
#define SCHC_AL_MAX_PACKET_SIZE 2048
#define IPV6_VERSION 6

static bool processing_required = false;

static enum {
    NO_EVENT = 0,
    CONNECTIVITY_AVAILABLE_EVENT = 1 << 0,
    IP_PACKET_AVAILABLE = 1 << 1,
    ONGOING_TRANSMISSION = 1 << 2
} event;

static int tun_fd = -1;
static char tun_name[IFNAMSIZ] = "";
static uint8_t net_buffer[SCHC_AL_MAX_PACKET_SIZE];
// static uint8_t uplink_buffer[SCHC_AL_MAX_PACKET_SIZE];
// static uint16_t uplink_size = 0;

static int run_cmd(const char *cmd) {
    const int rc = system(cmd);
    if (rc != 0) {
        PRINT_MSG("schc_al>command failed: %s\n", cmd);
        return -1;
    }
    return 0;
}

static void schc_al_tun_received_handler(void) {
    event |= IP_PACKET_AVAILABLE;
    processing_required = true;
}

int schc_al_init() {
    event = NO_EVENT;
    processing_required = false;

    // For now encoded in ascii
    if (!oscore_security_context_init((uint8_t*) OSCORE_MASTER_SECRET, OSCORE_MASTER_SECRET_LEN,
        (uint8_t*) OSCORE_MASTER_SALT, OSCORE_MASTER_SALT_LEN)) {
        return -1;
    }

    if (tun_fd >= 0) {
        return 0;
    }

    tun_fd = create_tun(TUN_NAME, tun_name, sizeof(tun_name));
    if (tun_fd < 0) {
        return -1;
    }

    char cmd[128];
    snprintf(cmd, sizeof(cmd), "ip link set dev %s up", tun_name);
    if (run_cmd(cmd) != 0) {
        close(tun_fd);
        tun_fd = -1;
        return -1;
    }

    snprintf(cmd, sizeof(cmd), "ip -6 addr add %s dev %s", IPv6_ADDR, tun_name);
    if (run_cmd(cmd) != 0) {
        close(tun_fd);
        tun_fd = -1;
        return -1;
    }

    // This forces all the traffic go through tun
    snprintf(cmd, sizeof(cmd), "ip -6 route replace default dev %s", tun_name);
    if (run_cmd(cmd) != 0) {
        close(tun_fd);
        tun_fd = -1;
        return -1;
    }

    if (!watch_fd_for_input(tun_fd, schc_al_tun_received_handler)) {
        PRINT_MSG("schc_al>watch_fd_for_input() failed\n");
        close(tun_fd);
        tun_fd = -1;
        return -1;
    }
    event |= CONNECTIVITY_AVAILABLE_EVENT;
    processing_required = true;

    return 0;
}

// void schc_al_set_processing_required(bool required) {
//     processing_required = required;
// }

bool schc_al_is_processing_required() {
    return processing_required;
}

static schc_al_process_status_t schc_al_send_down() {
    if (tun_fd < 0) {
        return SEND_DOWN_INTERNAL_ERROR;
    }

    if (event & ONGOING_TRANSMISSION) {
        return SEND_DOWN_BUSY;
    }

    ssize_t pkt_len;
    do {
        pkt_len = read(tun_fd, net_buffer, sizeof(net_buffer));
    } while (pkt_len < 0 && errno == EINTR);

    if (pkt_len < 0) {
        if (errno != EAGAIN && errno != EWOULDBLOCK) {
            PRINT_MSG("schc_al>recv() failed: %s\n", strerror(errno));
            return SEND_DOWN_INTERNAL_ERROR;
        }
        return SEND_DOWN_OK;
    }

    if (!is_ipv6_udp_packet(net_buffer, (size_t) pkt_len)) {
        PRINT_MSG("schc_al>discard non IPv6/UDP packet\n");
        return SEND_DOWN_OK;
    }

    // uint8_t* send_buf = downlink_buffer;
    // size_t send_buf_len = pkt_len;
#ifdef OSCORE_PROXY_ENABLED
    uint8_t* coap_packet = net_buffer + IPv6_HEADERS_BYTES + UDP_HEADERS_BYTES;
    uint32_t coap_packet_len = ipv6_udp_payload_len(net_buffer);
    uint32_t oscore_len = sizeof(net_buffer) - IPv6_HEADERS_BYTES - UDP_HEADERS_BYTES;
    if (is_coap_packet(coap_packet, coap_packet_len)) {
        // CoAP to OSCORE
        coap_oscore_res_t st = coap_to_oscore(coap_packet, coap_packet_len, coap_packet, &oscore_len);
#ifdef OSCORE_DROP_ON_ERROR
        if (st != CO_SUCCESS) {
            PRINT_MSG("schc_al>OSCORE encryption failed. Dropping packet\n");
            return SEND_DOWN_OSCORE_ERROR;
        }
#endif

        // I don't think I need to do this for the schc layer
        // if (st == CO_SUCCESS) {
        //     update_ip6_udp_len(net_buffer, coap_packet_len);
        //     update_udp_checksum(net_buffer);
        // }

        pkt_len = IPv6_HEADERS_BYTES + UDP_HEADERS_BYTES + oscore_len;
    }
#endif

    PRINT_MSG("Before fragmentation\n");
    PRINT_HEX_BUF(net_buffer, pkt_len);

    event |= ONGOING_TRANSMISSION;
    const net_status_t status = net_sendto(net_buffer, pkt_len);
    if (status != NET_SUCCESS) {
        PRINT_MSG("schc_al>net_sendto() failed (status %d)\n", status);
        event &= ~ONGOING_TRANSMISSION;
        return SEND_DOWN_INTERNAL_ERROR;
    }

    return SEND_DOWN_OK;
}

schc_al_process_status_t schc_al_process() {
    if (event & IP_PACKET_AVAILABLE) {
        event &= ~IP_PACKET_AVAILABLE;
        processing_required = false;
        return schc_al_send_down();
    }

    return 0;
}

static void net_transmission_result(net_status_t status, uint16_t error) {
    (void) error;
    if (status != NET_SUCCESS) {
        PRINT_MSG("schc_al>transmission failed (status %d)\n", status);
    } else {
        PRINT_MSG("schc_al>transmission success (status %d)\n", status);
    }
    event &= ~ONGOING_TRANSMISSION;
}

static int schc_al_forward_up(uint8_t *buffer, uint16_t data_size) {
    if (tun_fd < 0 || data_size == 0) {
        return -1;
    }

    if (!is_ipv6_udp_packet(buffer, data_size)) {
        PRINT_MSG("schc_al>discard non IPv6/UDP packet\n");
        return 0;
    }

#ifdef OSCORE_PROXY_ENABLED
    uint8_t* coap_packet = buffer + IPv6_HEADERS_BYTES + UDP_HEADERS_BYTES;
    uint32_t coap_packet_len = ipv6_udp_payload_len(buffer);
    if (is_coap_packet(coap_packet, data_size)) {
        // OSCORE to CoAP
        coap_oscore_res_t st = oscore_to_coap(coap_packet, coap_packet_len, buffer, &coap_packet_len);
        if (st != CO_NOT_OSCORE && st != CO_SUCCESS) {
            PRINT_MSG("schc_al>discard invalid OSCORE packet up\n");
            return -1;
        }
        if (st != CO_NOT_OSCORE) {
            update_ip6_udp_len(buffer, coap_packet_len);
            update_udp_checksum(buffer);
            data_size = IPv6_HEADERS_BYTES + UDP_HEADERS_BYTES + coap_packet_len;
        }
    }
#endif

    ssize_t sent_bytes = write(tun_fd, buffer, data_size);
    if (sent_bytes < 0) {
        PRINT_MSG("schc_al>write() failed: %s\n", strerror(errno));
        return -1;
    }

    return 0;
}

static void net_data_received(const uint8_t *buffer, uint16_t data_size, net_status_t status) {
    if (status != NET_SUCCESS) {
        return;
    }
    schc_al_forward_up(buffer, data_size);
}

static const net_callbacks_t schc_al_net_callbacks = {
    net_transmission_result,
    net_data_received
};

const net_callbacks_t *schc_al_get_net_callbacks(void) {
    return &schc_al_net_callbacks;
}

int schc_al_terminate() {
    if (tun_fd >= 0) {
        unwatch_fd_for_input(tun_fd);
        close(tun_fd);
        tun_fd = -1;
    }

    event = NO_EVENT;
    processing_required = false;
    return 0;
}
