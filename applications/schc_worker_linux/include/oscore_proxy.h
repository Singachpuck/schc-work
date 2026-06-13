#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

typedef enum
{
    CO_SUCCESS,
    CO_BUFFER_ERROR,
    CO_COAP_ERROR,
    CO_OSCORE_ERROR,
    CO_NOT_OSCORE,
    CO_MAX_SESSION_REACHED,
    CO_UNKNOWN_SESSION,
} coap_oscore_res_t;

bool is_coap_packet(const uint8_t* raw_coap, size_t pkt_len);

bool oscore_security_context_init(uint8_t *oscore_master_secret,
                                uint16_t oscore_master_secret_size, uint8_t *oscore_master_salt,
                                uint16_t oscore_master_salt_size);

coap_oscore_res_t coap_to_oscore(uint8_t *coap_packet, uint16_t coap_packet_size, uint8_t *out, uint32_t *out_size);

coap_oscore_res_t oscore_to_coap(uint8_t *oscore_packet, uint16_t oscore_packet_size, uint8_t *out, uint32_t *out_size);