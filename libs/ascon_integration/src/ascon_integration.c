
#include "common/crypto_wrapper.h"

#include <string.h>

#include "asconaead128/crypto_aead.h"
#include "asconhash256/crypto_hash.h"
#include "ascon_hmac.h"
#include "oscore/security_context.h"

#define ASCON_KEY_BYTES 16
#define ASCON_NONCE_BYTES 16

struct context * get_oscore_ctx();

enum err aead(enum aes_operation op, const struct byte_array *in,
        const struct byte_array *key, struct byte_array *nonce,
        const struct byte_array *aad, struct byte_array *out,
        struct byte_array *tag) {
    struct context* c = get_oscore_ctx();

    if (c == NULL) {
        return aead_failed;
    }

    uint8_t tag_len;
    switch (c->cc.aead_alg) {
        case OSCORE_ASCONAEAD128:
            tag_len = 16;
            break;
        case OSCORE_ASCONAEAD128_64:
            tag_len = 8;
            break;
        case OSCORE_ASCONAEAD128_32:
            tag_len = 4;
            break;
        default:
            return oscore_invalid_algorithm_aead;
    }

    if (key->len < ASCON_KEY_BYTES) {
        return aead_failed;
    }
    if (nonce->len < ASCON_NONCE_BYTES) {
        return aead_failed;
    }

    int st;
    if (op == ENCRYPT) {
        if (out->len < in->len) {
            return buffer_to_small;
        }
        if (tag->len < tag_len) {
            return aead_failed;
        }

        st = ascon_aead_encrypt_trunc(
            tag->ptr,
            out->ptr,
            in->ptr,
            in->len,
            aad->ptr,
            aad->len,
            nonce->ptr,
            key->ptr,
            tag_len * 8
        );
    } else {
        if (out->len < in->len - tag_len) {
            return buffer_to_small;
        }

        // tag is unusable with dynamic tag sizes, assuming ciphertext + tag
        st = ascon_aead_decrypt_trunc(
            out->ptr,
            in->ptr + in->len - tag_len,
            in->ptr,
            in->len - tag_len,
            aad->ptr,
            aad->len,
            nonce->ptr,
            key->ptr,
            tag_len * 8
        );
    }

    if (st < 0) {
        return aead_failed;
    }

    tag->len = tag_len;
    return ok;
}

enum err hkdf_extract(enum hash_alg alg, const struct byte_array *salt,
               struct byte_array *ikm, uint8_t *out) {
    if (alg != ASCON_HASH_256) {
        return crypto_operation_not_implemented;
    }
    int rc;

    if (salt->ptr == NULL || salt->len == 0) {
        uint8_t zero_salt[ASCON_HASH_SIZE] = { 0 };
        rc = hmac_ascon(out, zero_salt, sizeof(zero_salt),
                        ikm->ptr, ikm->len);
    } else {
        rc = hmac_ascon(out, salt->ptr, salt->len,
                        ikm->ptr, ikm->len);
    }

    if (rc != 0) {
        return hkdf_failed;
    }
    return ok;
}

enum err hkdf_expand(enum hash_alg alg, const struct byte_array *prk,
              const struct byte_array *info, struct byte_array *out) {
    if (alg != ASCON_HASH_256) {
        return crypto_operation_not_implemented;
    }

    /* "N = ceil(L/HashLen)" */
    uint8_t iterations = (out->len + ASCON_HASH_SIZE - 1) / ASCON_HASH_SIZE;

    /* "L length of output keying material in octets (<= 255*HashLen)" */
    if (iterations > 255) {
        return hkdf_failed;
    }

    uint8_t t[ASCON_HASH_SIZE] = { 0 };
    uint8_t msg[ASCON_HASH_SIZE + info->len + 1]; /* T(i-1) || info || i */

    for (uint8_t i = 1; i <= iterations; i++) {
        size_t msg_len = 0;

        /* T(i-1) — empty for the first iteration */
        if (i > 1) {
            memcpy(msg, t, ASCON_HASH_SIZE);
            msg_len += ASCON_HASH_SIZE;
        }

        /* || info */
        memcpy(msg + msg_len, info->ptr, info->len);
        msg_len += info->len;

        /* || i */
        msg[msg_len] = i;
        msg_len += 1;

        if (hmac_ascon(t, prk->ptr, prk->len, msg, msg_len) != 0) {
            memset(t, 0, sizeof(t));
            memset(msg, 0, sizeof(msg));
            return hkdf_failed;
        }

        /* copy full block, or remainder on last iteration */
        size_t copy_len = ((out->len - (i - 1) * ASCON_HASH_SIZE) < ASCON_HASH_SIZE)
                              ? (out->len % ASCON_HASH_SIZE)
                              : ASCON_HASH_SIZE;
        memcpy(&out->ptr[(i - 1) * ASCON_HASH_SIZE], t, copy_len);
    }

    memset(t,   0, sizeof(t));
    memset(msg, 0, sizeof(msg));
    return ok;
}

enum err hkdf_ascon_hash_256(struct byte_array *master_secret,
               struct byte_array *master_salt,
               struct byte_array *info, struct byte_array *out) {
    BYTE_ARRAY_NEW(prk, ASCON_HASH_SIZE, ASCON_HASH_SIZE);
    TRY(hkdf_extract(ASCON_HASH_256, master_salt, master_secret, prk.ptr));
    TRY(hkdf_expand(ASCON_HASH_256, &prk, info, out));
    return ok;
}

enum err hash(enum hash_alg alg, const struct byte_array *in,
           struct byte_array *out) {
    if (alg != ASCON_HASH_256) {
        return crypto_operation_not_implemented;
    }

    if (in->ptr == NULL || out->ptr == NULL) {
        return wrong_parameter;
    }

    if (out->len < ASCON_HASH_SIZE) {
        return buffer_to_small;
    }

    if (crypto_hash(out->ptr, in->ptr, (unsigned long long)in->len) != 0) {
        return sha_failed;
    }

    return ok;
}