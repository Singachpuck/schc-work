/**
 * @file fullsdkextapi.c
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
 * @author Flavien Moullec flavien@ackl.io
 *
 * Orange Labs extension API.
 */

#include <arpa/inet.h>
#include <stdlib.h>

#define MAX_DOWN_SESSIONS 10
#define NO_COMP_RULE_ID 0x1c

#include <fullsdkmgt.h>
#include <fullsdkmgtpriv.h>
#include <schccomp.h>

#include "outerrules.h"

uint8_t host_ipv6_addr[IPV6_ADDRESS_LENGTH_BYTES] = {0};
uint8_t host_udp_port[IP_PORT_LENGTH_BYTES] = {0};

uint8_t remote_ipv6_addr[IPV6_ADDRESS_LENGTH_BYTES] = {0};
uint8_t remote_udp_port[IP_PORT_LENGTH_BYTES] = {0};

static const rules_t* get_inner_rules() {
  // Initialize the inner rules array.
  static rules_t rules;
  init_rules(&rules, NULL, NO_COMP_RULE_ID);

  return &rules;
}

static const rules_t* get_outer_rules() {
#ifdef OUTERRULES_H
  return get_orangelabs_outer_rules();
#else
  // Initialize the outer rules array.
  static rules_t rules;
  init_rules(&rules, NULL, NO_COMP_RULE_ID);

  return &rules;
#endif
}

void net_set_host_static_ip(const char *ipv6_address)
{
  inet_pton(AF_INET6, ipv6_address, host_ipv6_addr);
}

void net_set_host_static_port(const char *udp_port)
{
  int port = atoi(udp_port);
  host_udp_port[0] = port / 256;
  host_udp_port[1] = port % 256;
}

void net_set_remote_static_ip(const char *ipv6_address)
{
  inet_pton(AF_INET6, ipv6_address, remote_ipv6_addr);
}

void net_set_remote_static_port(const char *udp_port)
{
  int port = atoi(udp_port);
  remote_udp_port[0] = port / 256;
  remote_udp_port[1] = port % 256;
}

mgt_status_t mgt_enable_extension_api(void)
{
  const rules_t *outer_rules = get_outer_rules();

  return mgt_set_rules(outer_rules, NULL);
}

static bool ext_compress(bit_buffer_t *output_bb_ptr,
                         bit_string_t *input_bs_ptr)
{
  (void)output_bb_ptr;
  (void)input_bs_ptr;

  return true;
}

mgt_status_t mgt_ext_oscore_inner_compression(uint8_t *out_buf,
                                              uint16_t out_buf_size,
                                              uint16_t *out_data_size,
                                              const uint8_t *in_data,
                                              uint16_t in_data_size)
{
  // Retrieve inner SCHC rules.
  const rules_t *rules = get_inner_rules();
  comp_callbacks_t callbacks = {.ext_compress = ext_compress};

  if (schc_oscore_inner_compress(rules, out_buf, out_buf_size, out_data_size,
                                 (uint8_t *)in_data, in_data_size,
                                 &callbacks) != COMP_SUCCESS)
  {
    return MGT_ERROR;
  }

  return MGT_SUCCESS;
}

mgt_status_t mgt_ext_oscore_inner_decompression(uint8_t *out_buf,
                                                uint16_t out_buf_size,
                                                uint16_t *out_data_size,
                                                const uint8_t *in_data,
                                                uint16_t in_data_size)
{
  // Retrieve inner SCHC rules.
  const rules_t *rules = get_inner_rules();

  if (schc_oscore_inner_decompress(rules, out_buf, out_buf_size, out_data_size,
                                   (uint8_t *)in_data,
                                   in_data_size) != COMP_SUCCESS)
  {
    return MGT_ERROR;
  }

  return MGT_SUCCESS;
}