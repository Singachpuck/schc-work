#ifndef ASCON_HMAC_H
#define ASCON_HMAC_H

#include <stdint.h>
#include <stddef.h>

#define ASCON_HASH_SIZE 32
#define ASCON_HMAC_BLOCK_SIZE 32

int hmac_ascon(
    uint8_t *out,
    const uint8_t *key,
    size_t keylen,
    const uint8_t *msg,
    size_t msglen);

#endif