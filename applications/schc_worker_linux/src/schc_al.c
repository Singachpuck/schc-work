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

#include "logging.h"

static const char *TAG = "SCHC-AL";

// TODO: Change to smaller
// #define SCHC_AL_MAX_PACKET_SIZE
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
static uint8_t net_buffer[IPv6_MAX_PACKET_SIZE];
// static uint8_t uplink_buffer[SCHC_AL_MAX_PACKET_SIZE];
// static uint16_t uplink_size = 0;

static int run_cmd(const char *cmd) {
    const int rc = system(cmd);
    if (rc != 0) {
        LOGERROR(TAG, "command failed: %s", cmd);
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

    LOGINFO(TAG, "Initializing SCHC adaptation layer");

    // For now encoded in ascii
    if (!oscore_security_context_init((uint8_t*) OSCORE_MASTER_SECRET, OSCORE_MASTER_SECRET_LEN,
        (uint8_t*) OSCORE_MASTER_SALT, OSCORE_MASTER_SALT_LEN)) {
      LOGERROR(TAG, "Failed to create OSCORE secutiry context");  
      return -1;
    }

    if (tun_fd >= 0) {
        return 0;
    }

    tun_fd = create_tun(TUN_NAME, tun_name, sizeof(tun_name));
    if (tun_fd < 0) {
      LOGERROR(TAG, "Failed to create tun");
      return -1;
    }

    char cmd[128];
    snprintf(cmd, sizeof(cmd), "ip link set dev %s up", tun_name);
    if (run_cmd(cmd) != 0) {
        close(tun_fd);
        tun_fd = -1;
        LOGERROR(TAG, "Failed to set device up");
        return -1;
    }

    snprintf(cmd, sizeof(cmd), "ip -6 addr add %s dev %s", IPv6_ADDR, tun_name);
    if (run_cmd(cmd) != 0) {
        close(tun_fd);
        tun_fd = -1;
        LOGERROR(TAG, "Failed to set device address");
        return -1;
    }

    // This forces all the traffic go through tun
    snprintf(cmd, sizeof(cmd), "ip -6 route replace default dev %s", tun_name);
    if (run_cmd(cmd) != 0) {
        close(tun_fd);
        tun_fd = -1;
        LOGERROR(TAG, "Failed to set IP route");
        return -1;
    }

    if (!watch_fd_for_input(tun_fd, schc_al_tun_received_handler)) {
        LOGERROR(TAG, "watch_fd_for_input() failed");
        close(tun_fd);
        tun_fd = -1;
        return -1;
    }
    event |= CONNECTIVITY_AVAILABLE_EVENT;
    processing_required = true;

    return 0;
}

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
            LOGERROR(TAG, "recv() failed: %s", strerror(errno));
            return SEND_DOWN_INTERNAL_ERROR;
        }
        return SEND_DOWN_OK;
    }

    if (!is_ipv6_udp_packet(net_buffer, (size_t) pkt_len)) {
        LOGWARN(TAG, "discard non IPv6/UDP packet");
        return SEND_DOWN_OK;
    }

    uint8_t* coap_packet = net_buffer + IPv6_HEADERS_BYTES + UDP_HEADERS_BYTES;
    uint32_t coap_packet_len = ipv6_udp_payload_len(net_buffer);
    if (!is_coap_packet(coap_packet, coap_packet_len)) {
#ifdef DROP_NON_COAP
        LOGWARN(TAG, "discard non CoAP packet");
        return SEND_DOWN_OK;
#endif
    } else {
#ifdef OSCORE_PROXY_ENABLED
        uint32_t oscore_len = sizeof(net_buffer) - IPv6_HEADERS_BYTES - UDP_HEADERS_BYTES;
        // CoAP to OSCORE
        coap_oscore_res_t st = coap_to_oscore(coap_packet, coap_packet_len, coap_packet, &oscore_len);
        if (st != CO_SUCCESS) {
            oscore_len = coap_packet_len;
#ifdef OSCORE_DROP_ON_ERROR
            LOGERROR(TAG, "OSCORE encryption failed. Dropping packet");
            return SEND_DOWN_OSCORE_ERROR;
#endif
        }

        // I don't think I need to do this for the schc layer
        // if (st == CO_SUCCESS) {
        //     update_ip6_udp_len(net_buffer, coap_packet_len);
        //     update_udp_checksum(net_buffer);
        // }

        pkt_len = IPv6_HEADERS_BYTES + UDP_HEADERS_BYTES + oscore_len;
#endif
    }

    LOGINFO(TAG, "Data before fragmentation");
    PRINT_HEX_BUF(net_buffer, pkt_len);

    event |= ONGOING_TRANSMISSION;
    const net_status_t status = net_sendto(net_buffer, pkt_len);
    if (status != NET_SUCCESS) {
        LOGERROR(TAG, "net_sendto() failed (status %d)", status);
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
        LOGERROR(TAG, "transmission failed (status %d)", status);
    } else {
        LOGINFO(TAG, "transmission success (status %d)", status);
    }
    event &= ~ONGOING_TRANSMISSION;
}

static int schc_al_forward_up(uint8_t *buffer, uint16_t data_size) {
    LOGINFO(TAG, "Forwarding up");

    if (tun_fd < 0 || data_size == 0) {
        return -1;
    }

    if (!is_ipv6_udp_packet(buffer, data_size)) {
        LOGWARN(TAG, "discard non IPv6/UDP packet");
        return 0;
    }

    uint8_t* coap_packet = buffer + IPv6_HEADERS_BYTES + UDP_HEADERS_BYTES;
    uint32_t coap_packet_len = ipv6_udp_payload_len(buffer);
    if (is_coap_packet(coap_packet, data_size)) {
#ifdef OSCORE_PROXY_ENABLED
        // OSCORE to CoAP
        coap_oscore_res_t st = oscore_to_coap(coap_packet, coap_packet_len, buffer, &coap_packet_len);
        if (st != CO_NOT_OSCORE && st != CO_SUCCESS) {
            LOGWARN(TAG, "discard invalid OSCORE packet up");
            return -1;
        }
        if (st != CO_NOT_OSCORE) {
            update_ip6_udp_len(buffer, coap_packet_len);
            update_udp_checksum(buffer);
            data_size = IPv6_HEADERS_BYTES + UDP_HEADERS_BYTES + coap_packet_len;
        }
#endif
    } else {
#ifdef DROP_NON_COAP
        LOGWARN(TAG, "discard non CoAP packet");
        return 0;
#endif
    }

    ssize_t sent_bytes = write(tun_fd, buffer, data_size);
    if (sent_bytes < 0) {
        LOGERROR(TAG, "write() failed: %s", strerror(errno));
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

    LOGINFO(TAG, "Terminating SCHC adaptation layer");

    event = NO_EVENT;
    processing_required = false;
    return 0;
}
