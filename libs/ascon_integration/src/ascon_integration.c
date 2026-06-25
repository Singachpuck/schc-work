
#include "common/crypto_wrapper.h"

#include "asconaead128/ascon.h"
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
