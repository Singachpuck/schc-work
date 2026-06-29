#include "ascon_hmac.h"

#include <string.h>

#include "ascon.h"

int hmac_ascon(
    uint8_t *out,
    const uint8_t *key,
    size_t keylen,
    const uint8_t *msg,
    size_t msglen)
{
    uint8_t k_ipad[ASCON_HMAC_BLOCK_SIZE];
    uint8_t k_opad[ASCON_HMAC_BLOCK_SIZE];
    uint8_t k0[ASCON_HMAC_BLOCK_SIZE];

    uint8_t inner_hash[ASCON_HASH_SIZE];

    ascon_state_t s = {0};
    uint8_t i;

    memset(k0, 0, sizeof(k0));

    /* Step 1: key normalization */
    if (keylen > ASCON_HMAC_BLOCK_SIZE) {
        /* hash long key */
        ascon_inithash(&s);
        ascon_absorb(&s, key, keylen);
        ascon_squeeze(&s, k0, ASCON_HASH_SIZE);
    } else {
        memcpy(k0, key, keylen);
    }

    /* Step 2: build ipad/opad */
    for (i = 0; i < ASCON_HMAC_BLOCK_SIZE; i++) {
        k_ipad[i] = k0[i] ^ 0x36;
        k_opad[i] = k0[i] ^ 0x5c;
    }

    /* Step 3: inner hash = H(k_ipad || msg) */

    ascon_inithash(&s);

    ascon_absorb(&s, k_ipad, ASCON_HMAC_BLOCK_SIZE);
    ascon_absorb(&s, msg, msglen);

    ascon_squeeze(&s, inner_hash, ASCON_HASH_SIZE);


    /* Step 4: outer hash = H(k_opad || inner_hash) */
    ascon_inithash(&s);

    ascon_absorb(&s, k_opad, ASCON_HMAC_BLOCK_SIZE);
    ascon_absorb(&s, inner_hash, ASCON_HASH_SIZE);

    ascon_squeeze(&s, out, ASCON_HASH_SIZE);

    memset(k_ipad,    0, sizeof(k_ipad));
    memset(k_opad,    0, sizeof(k_opad));
    memset(k0,        0, sizeof(k0));
    memset(inner_hash,0, sizeof(inner_hash));
    memset(&s,        0, sizeof(s));
    return 0;
}