/**
 * @file uoscore.c
 * @copyright
 * Copyright (c) 2018-2023 ACKLIO SAS
 * Copyright (c) 2024 ACTILITY SA - All Rights Reserved
 * 
 * This file is part of lab.SCHC FullSDK.
 * 
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 * 
 * @author: Hadi Bereksi hadi-ilies.bereksi-reguig@ackl.io
 *
 * Modified by Dmytro Ochkas dmytro.ochkas@imt-atlantique.fr
 */

#include <string.h>
#include <stdlib.h>
// #include "libcoap.h"
#include "oscore.h"
// #include "uoscore.h"
// #include "common.h"
#include "net/udp.h"

#include <fullsdkmgt.h>
#include <fullsdkextapi.h>

#include "schc_al_params.h"
#include "oscore_proxy.h"
// #include "platform.h"

static uint8_t *SENDER_ID = NULL;
static uint8_t SENDER_ID_LEN = 0;

static uint8_t RECIPIENT_ID[1] = {0x01};
static uint8_t RECIPIENT_ID_LEN = sizeof(RECIPIENT_ID);

static uint8_t *ID_CONTEXT = NULL;
static uint8_t ID_CONTEXT_LEN = 0;

static struct context osc_ctx;

struct context * get_oscore_ctx() {
    return &osc_ctx;
}

#ifdef REGULAR_COMP
static bool
oscore_msg_inner_compression(uint8_t *plaintext, uint32_t *plaintext_size)
{
    uint8_t out_inner_comp_buf[OSCORE_INNER_MAX_SIZE] = {0};
    uint16_t out_data_size = 0;
    mgt_status_t status = mgt_ext_oscore_inner_compression(
        out_inner_comp_buf, sizeof(out_inner_comp_buf), &out_data_size,
        plaintext, *plaintext_size);
    if (status != MGT_SUCCESS)
    {
        printf("Inner compression error in SDK: %d\n", status);
        return false;
    }

    PRINT_ARRAY("Inner compressed packet" , out_inner_comp_buf, out_data_size);

    // Move the compressed OSCORE Plaintext into the input buffer.
    if (memcpy(plaintext, out_inner_comp_buf, out_data_size) == NULL)
        return false;
    *plaintext_size = out_data_size;
    return true;
}

static bool
oscore_msg_inner_decompression(uint8_t *plaintext, uint32_t *plaintext_size)
{
    uint8_t out_inner_decomp_buf[OSCORE_INNER_MAX_SIZE] = {0};
    uint16_t out_data_size;
    mgt_status_t status = mgt_ext_oscore_inner_decompression(
        out_inner_decomp_buf, sizeof(out_inner_decomp_buf), &out_data_size,
        plaintext, *plaintext_size);
    if (status != MGT_SUCCESS)
    {
        printf("Inner decompression error in SDK: %d\n", status);
        return false;
    }
    PRINT_ARRAY("Inner decompressed packet", out_inner_decomp_buf, out_data_size);

    // Move the OSCORE Plaintext message into the input buffer.
    if (memcpy(plaintext, out_inner_decomp_buf, out_data_size) == NULL)
        return false;
    *plaintext_size = out_data_size;
    return true;
}
#endif

#ifdef PATTERN_COMP

static bool oscore_msg_inner_compression(uint8_t *plaintext, uint32_t *plaintext_size)
{
    printf("------------- Plaintext ------------\n");
    dump_buf(plaintext, *plaintext_size);

    for (uint16_t i = 0; i < nb_pattern_table; i++)
    {
        if (pattern_compression_match(&pattern_tables[i], plaintext, plaintext_size))
        {
            printf("\nPATTERN COMPRESSION MATCHED: %d\n", pattern_tables[i].rule_id);
            return true;
        }
    }
    //add rule_id
    uint8_t out_inner_comp_buf[MAX_PACKET_SIZE_BYTES] = {0};
    uint16_t out_data_size = *plaintext_size + sizeof(uint8_t);

    out_inner_comp_buf[0] = NO_COMPRESSION_RULE_ID;
    if (memcpy(out_inner_comp_buf + sizeof(uint8_t), plaintext, *plaintext_size) == NULL)
        return false;
    printf("NO PATTERN COMPRESSION HAS MATCHED\n");
    printf("------------- Inner NOT Compressed packet ------------\n");
    dump_buf(out_inner_comp_buf, out_data_size);

    // Move the compressed OSCORE Plaintext into the input buffer.
    if (memcpy(plaintext, out_inner_comp_buf, out_data_size) == NULL)
        return false;
    *plaintext_size = out_data_size;
    return true;
}

static bool oscore_msg_inner_decompression(uint8_t *plaintext, uint32_t *plaintext_size)
{
    printf("------------- Inner compressed packet ------------\n");
    dump_buf(plaintext, *plaintext_size);

    if (plaintext[0] == NO_COMPRESSION_RULE_ID)
    {
        //remove rule_id
        uint8_t *tmp = plaintext + sizeof(uint8_t);
        *plaintext_size = *plaintext_size - sizeof(uint8_t);
        memcpy(plaintext, tmp, *plaintext_size);

        printf("------------- Inner NOT DEcompressed packet ------------\n");
        dump_buf(plaintext, *plaintext_size);
    }
    else
    {
        for (uint16_t i = 0; i < nb_pattern_table; i++)
        {
            if (pattern_decompression_match(&pattern_tables[i], plaintext, plaintext_size))
            {
                printf("\nPATTERN DECOMPRESSION MATCHED: %d\n", pattern_tables[i].rule_id);
                return true;
            }
        }
    }
    return true;
}

#endif

static bool oscore_inner_schc_enabled = true;

void disable_inner_compression(void)
{
    oscore_inner_schc_enabled = false;
}

static coap_oscore_res_t uoscore_err_to_res(enum err error) {
    switch (error) {
        case ok:
            return CO_SUCCESS;
        case buffer_to_small:
            return CO_BUFFER_ERROR;
        case not_oscore_pkt:
            return CO_NOT_OSCORE;
        default:
            return CO_OSCORE_ERROR;
    }
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

bool oscore_security_context_init(uint8_t *oscore_master_secret,
                                uint16_t oscore_master_secret_size, uint8_t *oscore_master_salt,
                                uint16_t oscore_master_salt_size)
{
    /*OSCORE contex initialization*/
    struct oscore_init_params params_sender =
    {
        {
            oscore_master_secret_size,
            oscore_master_secret
        },
        {
            SENDER_ID_LEN,
            SENDER_ID
        },
        {
            RECIPIENT_ID_LEN,
            RECIPIENT_ID
        },
        {
            ID_CONTEXT_LEN,
            ID_CONTEXT
        },
        {
            oscore_master_salt_size,
            oscore_master_salt
        },
        OSCORE_ASCONAEAD128_32,
        OSCORE_ASCON_256
    };
    enum err r = oscore_context_init(&params_sender, &osc_ctx);
    if (r != ok)
    {
        printf("Error : during establishing an OSCORE security context!\n");
        return false;
    }
    return true;
}

coap_oscore_res_t coap_to_oscore(uint8_t *coap_packet, uint16_t coap_packet_size, uint8_t *out, uint32_t *out_size)
{
    plaintext_cb_t inner_cb = oscore_inner_schc_enabled ? oscore_msg_inner_compression : NULL;
    enum err r = coap2oscore(coap_packet, coap_packet_size, out, out_size, &osc_ctx, inner_cb);
    if (r != ok)
    {
        printf("Error : coap2oscore: %d\n", r);
        return uoscore_err_to_res(r);
    }
    return CO_SUCCESS;
}

coap_oscore_res_t oscore_to_coap(uint8_t *oscore_packet, uint16_t oscore_packet_size, uint8_t *out, uint32_t *out_size)
{
    plaintext_cb_t inner_cb = oscore_inner_schc_enabled ? oscore_msg_inner_decompression : NULL;
    //convert oscore to coap
    enum err r = oscore2coap(oscore_packet, oscore_packet_size, out, out_size, &osc_ctx, inner_cb);
    if (r != ok)
    {
        printf("Error : oscore2coap: %d\n", r);
        return uoscore_err_to_res(r);
    }
    return CO_SUCCESS;
}


// NVM storage implementation, move somewhere else
#define MAX_NVM_LINE_LENGTH 256 // Max length for a line in the NVM file

#include "base64.h"

bool nvm_key_equals(const struct nvm_key_t *nvm_key_1, const struct nvm_key_t *nvm_key_2) {
    bool res = array_equals(&nvm_key_1->sender_id, &nvm_key_2->sender_id);
    if (!res) {
        return false;
    }

    res = array_equals(&nvm_key_1->recipient_id, &nvm_key_2->recipient_id);
    if (!res) {
        return false;
    }

    res = array_equals(&nvm_key_1->id_context, &nvm_key_2->id_context);
    if (!res) {
        return false;
    }

    return true;
}

char *nvm_key_base64_encode(const struct nvm_key_t *nvm_key) {
  uint8_t nvm_key_bytes_len = nvm_key->sender_id.len + nvm_key->recipient_id.len + nvm_key->id_context.len + sizeof(
                                uint32_t) * 3;
  uint8_t nvm_key_bytes[nvm_key_bytes_len];
  uint8_t *nvm_ptr = nvm_key_bytes;
  struct byte_array bytes[3];
  bytes[0] = nvm_key->sender_id;
  bytes[1] = nvm_key->recipient_id;
  bytes[2] = nvm_key->id_context;
  for (uint16_t i = 0; i < 3; ++i) {
    memcpy(nvm_ptr, (uint8_t *) &bytes[i].len, sizeof(uint32_t));
    nvm_ptr += sizeof(uint32_t);
    memcpy(nvm_key_bytes, (uint8_t *) &bytes[i].ptr, bytes[i].len);
    nvm_ptr += bytes[i].len;
  }

  return b64_encode(nvm_key_bytes, nvm_key_bytes_len);
}

enum err nvm_key_base64_decode(const char *b64_input, struct nvm_key_t *nvm_key) {
  size_t decoded_len = 0;
  uint8_t *decoded_bytes = b64_decode(b64_input, &decoded_len);
  if (decoded_bytes == NULL) {
    return -1;
  }

  struct byte_array bytes[3];
  bytes[0] = nvm_key->sender_id;
  bytes[1] = nvm_key->recipient_id;
  bytes[2] = nvm_key->id_context;

  uint8_t *current_ptr = decoded_bytes;
  size_t remaining_len = decoded_len;

  for (uint16_t i = 0; i < 3; ++i) {
    // Decode sender_id
    if (remaining_len < sizeof(uint32_t)) { goto decode_error; }
    uint32_t curr_len = *current_ptr;
    if (curr_len != bytes[i].len) {
      goto decode_error;
    }
    current_ptr += sizeof(uint32_t);
    remaining_len -= sizeof(uint32_t);
    if (remaining_len < bytes[i].len) { goto decode_error; }
    bytes[i].ptr = current_ptr;
    current_ptr += bytes[i].len;
    remaining_len -= bytes[i].len;
  }

  free(decoded_bytes);
  return ok;

decode_error:
  free(decoded_bytes);
  return oscore_ssn_overflow;
}

enum err nvm_write_ssn(const struct nvm_key_t *nvm_key, uint64_t ssn) {
  FILE *file = fopen(NVM_FILENAME, "w");
  if (file == NULL) {
    fprintf(stderr, "Error: nvm_write_ssn(): Could not open NVM file %s for writing.\n", NVM_FILENAME);
    return oscore_ssn_overflow;
  }

  char *nvm_key_base64 = nvm_key_base64_encode(nvm_key);

  fprintf(file, "NVM_KEY=%s\n", nvm_key_base64);
  fprintf(file, "OSCORE_SSN=%lu\n", ssn);

  free(nvm_key_base64);
  fclose(file);
  return ok;
}

enum err nvm_read_ssn(const struct nvm_key_t *nvm_key, uint64_t *ssn) {
  FILE *file = fopen(NVM_FILENAME, "r");
  if (file == NULL) {
    // File might not exist yet, or other error. Treat as key not found.
    *ssn = 0;
    enum err write_res = nvm_write_ssn(nvm_key, *ssn);
    if (write_res != ok) {
        fprintf(stderr, "Error: nvm_read_ssn(): Could not create NVM file %s.\n", NVM_FILENAME);
        return write_res;
    }
    return ok;
  }

  char line[MAX_NVM_LINE_LENGTH];
  char read_nvm_key[MAX_NVM_LINE_LENGTH] = {0};
  uint32_t read_ssn = 0;
  bool nvm_key_found = false;
  bool ssn_found = false;

  while (fgets(line, sizeof(line), file) != NULL) {
    // Remove newline character if present
    line[strcspn(line, "\n")] = 0;

    if (sscanf(line, "NVM_KEY=%s", read_nvm_key) == 1) {
      nvm_key_found = true;
    } else if (sscanf(line, "OSCORE_SSN=%u", &read_ssn) == 1) {
      ssn_found = true;
    }
  }
  fclose(file);

    // TODO: compare the nvm keys

  if (nvm_key_found && ssn_found) {
    *ssn = read_ssn;
    return ok;
  }

  return oscore_ssn_overflow; // Key not found, or SSN not found, or nvm_key mismatch
}