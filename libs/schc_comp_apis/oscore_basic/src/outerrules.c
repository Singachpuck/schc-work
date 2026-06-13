/**
 * @file outerrules.c
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
 * @author Thibaut Artis thibaut.artis@ackl.io
 *
 * Orange Labs outer compression rules.
 */

#include <stdlib.h>

#include "fullsdkextapi.h"
#include "outerrules.h"
#include "parsercoap.h"
#include "parserudp.h"
#include "rule.h"

#define NO_COMP_RULE_ID 150

// Compression rule ID
#define COAP_OSCORE_REQUEST_RULE_ID 56
#define COAP_OSCORE_RESPONSE_RULE_ID 57
#define COAP_OSCORE_OBSERVE_REQUEST_RULE_ID 58
#define COAP_OSCORE_OBSERVE_RESPONSE_RULE_ID 59
#define COAP_OSCORE_OBSERVE_NOTIF_RULE_ID 60
#define IP_UDP_RULE_ID 61
#define EDHOC_COAP_REQUEST_RULE_ID 69
#define EDHOC_COAP_RESPONSE_RULE_ID 70

static void add_ip_fields(rule_t *rule)
{
  // IPv6 layer
  // Version
  static uint8_t ipv6_version = 0x60; // Only 4 MSB bits are used.
  static target_value_t ipv6_version_tv = {TV_BIT_STRING,
                                           {{&ipv6_version, 0, 4}}};
  // Traffic Class.
  static uint8_t ipv6_traffic_class = 0;
  static target_value_t ipv6_traffic_class_tv = {TV_BIT_STRING,
                                                 {{&ipv6_traffic_class, 0, 8}}};
  //  Flow Label.
  static uint8_t ipv6_flow_label[] = {0x12, 0x34,
                                      0x50}; // Only 20 MSB bits are used.
  static target_value_t ipv6_flow_label_tv = {TV_BIT_STRING,
                                              {{ipv6_flow_label, 0, 20}}};
  // Next Header for UDP.
  static uint8_t ipv6_next_header_udp = 17; // For UDP.
  static target_value_t ipv6_next_header_udp_tv = {
      TV_BIT_STRING, {{&ipv6_next_header_udp, 0, 8}}};

  // Hop Limit.
  static uint8_t ipv6_hop_limit = 64;
  static target_value_t ipv6_hop_limit_tv = {TV_BIT_STRING,
                                             {{&ipv6_hop_limit, 0, 8}}};

  // Device Prefix.
  static target_value_t ipv6_prefix_dev_tv = {TV_BIT_STRING,
                                              {{host_ipv6_addr, 0, 64}}};
  // Device IID.
  static target_value_t ipv6_iid_dev_tv = {TV_BIT_STRING,
                                           {{host_ipv6_addr + 8, 0, 64}}};

  // Remote app prefix.
  static target_value_t remote_ipv6_prefix_tv = {TV_BIT_STRING,
                                                 {{remote_ipv6_addr, 0, 64}}};
  // Remote IID.
  static target_value_t remote_ipv6_iid_tv = {TV_BIT_STRING,
                                              {{remote_ipv6_addr + 8, 0, 64}}};

  // Rule entry definitions for IPv6 layer.
  static rule_field_t rule_field_00 = {
      FID_IPV6_VERSION, 1,   DIR_BI,      &ipv6_version_tv, 4,
      MO_EQUAL,         {0}, CDA_NOT_SENT};
  static rule_field_t rule_field_01 = {FID_IPV6_TRAFFIC_CLASS,
                                       1,
                                       DIR_BI,
                                       &ipv6_traffic_class_tv,
                                       8,
                                       MO_EQUAL,
                                       {0},
                                       CDA_NOT_SENT};
  static rule_field_t rule_field_02 = {
      FID_IPV6_FLOW_LABEL, 1,   DIR_BI,      &ipv6_flow_label_tv, 20,
      MO_IGNORE,           {0}, CDA_NOT_SENT};
  static rule_field_t rule_field_03 = {
      FID_IPV6_PAYLOAD_LENGTH, 1, DIR_BI, NULL, 16, MO_IGNORE, {0},
      CDA_COMPUTE_LENGTH};
  static rule_field_t rule_field_04 = {FID_IPV6_NEXT_HEADER,
                                       1,
                                       DIR_BI,
                                       &ipv6_next_header_udp_tv,
                                       8,
                                       MO_EQUAL,
                                       {0},
                                       CDA_NOT_SENT};
  static rule_field_t rule_field_05 = {
      FID_IPV6_HOP_LIMIT, 1,   DIR_BI,      &ipv6_hop_limit_tv, 8,
      MO_IGNORE,          {0}, CDA_NOT_SENT};
  static rule_field_t rule_field_06 = {
      FID_IPV6_PREFIX_DEV, 1, DIR_BI, &ipv6_prefix_dev_tv, 64, MO_EQUAL, {0},
      CDA_NOT_SENT};
  static rule_field_t rule_field_07 = {
      FID_IPV6_IID_DEV, 1,   DIR_BI,      &ipv6_iid_dev_tv, 64,
      MO_EQUAL,         {0}, CDA_NOT_SENT};
  static rule_field_t rule_field_08 = {
      FID_IPV6_PREFIX_APP, 1, DIR_BI, &remote_ipv6_prefix_tv, 64, MO_EQUAL, {0},
      CDA_NOT_SENT};
  static rule_field_t rule_field_09 = {
      FID_IPV6_IID_APP, 1,   DIR_BI,      &remote_ipv6_iid_tv, 64,
      MO_EQUAL,         {0}, CDA_NOT_SENT};

  add_rule_field(rule, &rule_field_00);
  add_rule_field(rule, &rule_field_01);
  add_rule_field(rule, &rule_field_02);
  add_rule_field(rule, &rule_field_03);
  add_rule_field(rule, &rule_field_04);
  add_rule_field(rule, &rule_field_05);
  add_rule_field(rule, &rule_field_06);
  add_rule_field(rule, &rule_field_07);
  add_rule_field(rule, &rule_field_08);
  add_rule_field(rule, &rule_field_09);
}

static void add_udp_fields(rule_t *rule)
{
  static target_value_t remote_udp_port_tv = {TV_BIT_STRING,
                                              {{remote_udp_port, 0, 16}}};

  static target_value_t host_udp_port_tv = {TV_BIT_STRING,
                                            {{host_udp_port, 0, 16}}};

  static rule_field_t rule_field_10 = {
      FID_UDP_PORT_DEV, 1,   DIR_BI,      &host_udp_port_tv, 16,
      MO_EQUAL,         {0}, CDA_NOT_SENT};
  static rule_field_t rule_field_11 = {
      FID_UDP_PORT_APP, 1,   DIR_BI,      &remote_udp_port_tv, 16,
      MO_EQUAL,         {0}, CDA_NOT_SENT};
  // UDP Length.
  static rule_field_t rule_field_12 = {
      FID_UDP_LENGTH, 1, DIR_BI, NULL, 16, MO_IGNORE, {0}, CDA_COMPUTE_LENGTH};
  // UDP Checksum.
  static rule_field_t rule_field_13 = {
      FID_UDP_CHECKSUM,    1, DIR_BI, NULL, 16, MO_IGNORE, {0},
      CDA_COMPUTE_CHECKSUM};

  add_rule_field(rule, &rule_field_10);
  add_rule_field(rule, &rule_field_11);
  add_rule_field(rule, &rule_field_12);
  add_rule_field(rule, &rule_field_13);
}

const rules_t *get_orangelabs_outer_rules(void)
{
  // EDHOC uri-path
  static uint8_t uri_path[] = ".well-known/edhoc";
  static target_value_t edhoc_coap_uri_path_tv = {
      TV_BIT_STRING, {{uri_path, 0, (sizeof(uri_path) - 1) * 8}}};
  static rule_field_t rule_field_edhoc_uri = {
      FID_COAP_URI_PATH, 1,   DIR_BI,      &edhoc_coap_uri_path_tv, 0,
      MO_EQUAL,          {0}, CDA_NOT_SENT};

  // CoAP fields
  /* ---------------------------------------------------------------------------------*/
  // Version

  static uint8_t coap_version = 0x01;
  static target_value_t coap_version_tv = {TV_BIT_STRING,
                                           {{&coap_version, 6, 2}}};

  static rule_field_t rule_field_14 = {
      FID_COAP_VERSION, 1,   DIR_BI,      &coap_version_tv, 2,
      MO_EQUAL,         {0}, CDA_NOT_SENT};

  /* ---------------------------------------------------------------------------------*/
  // Type

  static uint8_t con_coap_type = 0b00;
  static target_value_t con_coap_type_tv = {TV_BIT_STRING,
                                            {{&con_coap_type, 6, 2}}};

  static uint8_t non_con_coap_type = 0b01;
  static target_value_t non_con_coap_type_tv = {TV_BIT_STRING,
                                                {{&non_con_coap_type, 6, 2}}};

  static uint8_t ack_coap_type = 0b10;
  static target_value_t ack_coap_type_tv = {TV_BIT_STRING,
                                            {{&ack_coap_type, 6, 2}}};

  static rule_field_t rule_field_15_con = {
      FID_COAP_TYPE, 1,   DIR_BI,      &con_coap_type_tv, 2,
      MO_EQUAL,      {0}, CDA_NOT_SENT};

  static rule_field_t rule_field_15_non_con = {
      FID_COAP_TYPE, 1,   DIR_BI,      &non_con_coap_type_tv, 2,
      MO_EQUAL,      {0}, CDA_NOT_SENT};
  static rule_field_t rule_field_15_ack = {
      FID_COAP_TYPE, 1,   DIR_BI,      &ack_coap_type_tv, 2,
      MO_EQUAL,      {0}, CDA_NOT_SENT};

  /* ---------------------------------------------------------------------------------*/
  // Token length

  static uint8_t coap_tkl = 2;
  static target_value_t coap_tkl_tv = {TV_BIT_STRING, {{&coap_tkl, 4, 4}}};
  static rule_field_t rule_field_16 = {
      FID_COAP_TOKEN_LENGTH, 1, DIR_BI, &coap_tkl_tv, 4, MO_EQUAL, {0},
      CDA_NOT_SENT};

  /* ---------------------------------------------------------------------------------*/
  // Code

  static uint8_t post_coap_code = COAP_POST;
  static target_value_t post_coap_code_tv = {TV_BIT_STRING,
                                             {{&post_coap_code, 0, 8}}};
  static uint8_t fetch_coap_code = COAP_FETCH;
  static target_value_t fetch_coap_code_tv = {TV_BIT_STRING,
                                              {{&fetch_coap_code, 0, 8}}};
  static uint8_t changed_coap_code = COAP_CHANGED;
  static target_value_t changed_coap_code_tv = {TV_BIT_STRING,
                                                {{&changed_coap_code, 0, 8}}};
  static uint8_t content_coap_code = COAP_CONTENT;
  static target_value_t content_coap_code_tv = {TV_BIT_STRING,
                                                {{&content_coap_code, 0, 8}}};

  static rule_field_t rule_field_17_post = {
      FID_COAP_CODE, 1,   DIR_BI,      &post_coap_code_tv, 8,
      MO_EQUAL,      {0}, CDA_NOT_SENT};

  static rule_field_t rule_field_17_fetch = {
      FID_COAP_CODE, 1,   DIR_BI,      &fetch_coap_code_tv, 8,
      MO_EQUAL,      {0}, CDA_NOT_SENT};
  static rule_field_t rule_field_17_changed = {
      FID_COAP_CODE, 1,   DIR_BI,      &changed_coap_code_tv, 8,
      MO_EQUAL,      {0}, CDA_NOT_SENT};
  static rule_field_t rule_field_17_content = {
      FID_COAP_CODE, 1,   DIR_BI,      &content_coap_code_tv, 8,
      MO_EQUAL,      {0}, CDA_NOT_SENT};

  /* ---------------------------------------------------------------------------------*/
  // Message ID

  static rule_field_t rule_field_18 = {
      FID_COAP_MSG_ID, 1, DIR_BI, NULL, 16, MO_IGNORE, {0}, CDA_VALUE_SENT};

  /* ---------------------------------------------------------------------------------*/
  // Token

  static rule_field_t rule_field_19 = {
      FID_COAP_TOKEN, 1, DIR_BI, NULL, 16, MO_IGNORE, {0}, CDA_VALUE_SENT};

  /* ---------------------------------------------------------------------------------*/
  // Observe option

  static uint8_t empty_option = 0x00;

  static target_value_t empty_option_tv = {TV_BIT_STRING,
                                           {{&empty_option, 0, 0}}};

  static rule_field_t rule_field_20_observe = {
      FID_COAP_OBSERVE, 1,   DIR_BI,      &empty_option_tv, 0,
      MO_EQUAL,         {0}, CDA_NOT_SENT};
  static rule_field_t rule_field_20_observe_notif = {
      FID_COAP_OBSERVE, 1, DIR_BI, NULL, 0, MO_IGNORE, {0}, CDA_VALUE_SENT};

  /* ---------------------------------------------------------------------------------*/
  // OSCORE option

  static uint8_t oscore_option_request = 0x09;

  static target_value_t oscore_option_request_tv = {
      TV_BIT_STRING, {{&oscore_option_request, 0, 8}}};

  static rule_field_t rule_field_21_request = {
      FID_COAP_OSCORE, 1,   DIR_BI, &oscore_option_request_tv, 0,
      MO_MSB,          {8}, CDA_LSB};
  static rule_field_t rule_field_21_response = {
      FID_COAP_OSCORE, 1,   DIR_BI,      &empty_option_tv, 0,
      MO_EQUAL,        {0}, CDA_NOT_SENT};
  static rule_field_t rule_field_21_observe_notif = {
      FID_COAP_OSCORE, 1, DIR_BI, NULL, 0, MO_IGNORE, {0}, CDA_VALUE_SENT};

  /* ---------------------------------------------------------------------------------*/

  static rule_field_t *rule_fields_1[21];
  static rule_t ipv6_coap_oscore_request_rule;
  init_rule(&ipv6_coap_oscore_request_rule, COAP_OSCORE_REQUEST_RULE_ID,
            STACK_IPV6_UDP_COAP, rule_fields_1);

  add_ip_fields(&ipv6_coap_oscore_request_rule);
  add_udp_fields(&ipv6_coap_oscore_request_rule);

  add_rule_field(&ipv6_coap_oscore_request_rule, &rule_field_14);
  add_rule_field(&ipv6_coap_oscore_request_rule, &rule_field_15_con);
  add_rule_field(&ipv6_coap_oscore_request_rule, &rule_field_16);
  add_rule_field(&ipv6_coap_oscore_request_rule, &rule_field_17_post);
  add_rule_field(&ipv6_coap_oscore_request_rule, &rule_field_18);
  add_rule_field(&ipv6_coap_oscore_request_rule, &rule_field_19);
  add_rule_field(&ipv6_coap_oscore_request_rule, &rule_field_21_request);

  static rule_field_t *rule_fields_2[21];
  static rule_t ipv6_coap_oscore_response_rule;
  init_rule(&ipv6_coap_oscore_response_rule, COAP_OSCORE_RESPONSE_RULE_ID,
            STACK_IPV6_UDP_COAP, rule_fields_2);

  add_ip_fields(&ipv6_coap_oscore_response_rule);
  add_udp_fields(&ipv6_coap_oscore_response_rule);

  add_rule_field(&ipv6_coap_oscore_response_rule, &rule_field_14);
  add_rule_field(&ipv6_coap_oscore_response_rule, &rule_field_15_ack);
  add_rule_field(&ipv6_coap_oscore_response_rule, &rule_field_16);
  add_rule_field(&ipv6_coap_oscore_response_rule, &rule_field_17_changed);
  add_rule_field(&ipv6_coap_oscore_response_rule, &rule_field_18);
  add_rule_field(&ipv6_coap_oscore_response_rule, &rule_field_19);
  add_rule_field(&ipv6_coap_oscore_response_rule, &rule_field_21_response);

  static rule_field_t *rule_fields_3[22];
  static rule_t ipv6_coap_oscore_observe_request_rule;
  init_rule(&ipv6_coap_oscore_observe_request_rule,
            COAP_OSCORE_OBSERVE_REQUEST_RULE_ID, STACK_IPV6_UDP_COAP,
            rule_fields_3);

  add_ip_fields(&ipv6_coap_oscore_observe_request_rule);
  add_udp_fields(&ipv6_coap_oscore_observe_request_rule);

  add_rule_field(&ipv6_coap_oscore_observe_request_rule, &rule_field_14);
  add_rule_field(&ipv6_coap_oscore_observe_request_rule, &rule_field_15_con);
  add_rule_field(&ipv6_coap_oscore_observe_request_rule, &rule_field_16);
  add_rule_field(&ipv6_coap_oscore_observe_request_rule, &rule_field_17_fetch);
  add_rule_field(&ipv6_coap_oscore_observe_request_rule, &rule_field_18);
  add_rule_field(&ipv6_coap_oscore_observe_request_rule, &rule_field_19);
  add_rule_field(&ipv6_coap_oscore_observe_request_rule,
                 &rule_field_20_observe);
  add_rule_field(&ipv6_coap_oscore_observe_request_rule,
                 &rule_field_21_request);

  static rule_field_t *rule_fields_4[22];
  static rule_t ipv6_coap_oscore_observe_response_rule;
  init_rule(&ipv6_coap_oscore_observe_response_rule,
            COAP_OSCORE_OBSERVE_RESPONSE_RULE_ID, STACK_IPV6_UDP_COAP,
            rule_fields_4);

  add_ip_fields(&ipv6_coap_oscore_observe_response_rule);
  add_udp_fields(&ipv6_coap_oscore_observe_response_rule);

  add_rule_field(&ipv6_coap_oscore_observe_response_rule, &rule_field_14);
  add_rule_field(&ipv6_coap_oscore_observe_response_rule, &rule_field_15_ack);
  add_rule_field(&ipv6_coap_oscore_observe_response_rule, &rule_field_16);
  add_rule_field(&ipv6_coap_oscore_observe_response_rule,
                 &rule_field_17_content);
  add_rule_field(&ipv6_coap_oscore_observe_response_rule, &rule_field_18);
  add_rule_field(&ipv6_coap_oscore_observe_response_rule, &rule_field_19);
  add_rule_field(&ipv6_coap_oscore_observe_response_rule,
                 &rule_field_20_observe);
  add_rule_field(&ipv6_coap_oscore_observe_response_rule,
                 &rule_field_21_response);

  static rule_field_t *rule_fields_5[22];
  static rule_t ipv6_coap_oscore_observe_notif_rule;
  init_rule(&ipv6_coap_oscore_observe_notif_rule,
            COAP_OSCORE_OBSERVE_NOTIF_RULE_ID, STACK_IPV6_UDP_COAP,
            rule_fields_5);

  add_ip_fields(&ipv6_coap_oscore_observe_notif_rule);
  add_udp_fields(&ipv6_coap_oscore_observe_notif_rule);

  add_rule_field(&ipv6_coap_oscore_observe_notif_rule, &rule_field_14);
  add_rule_field(&ipv6_coap_oscore_observe_notif_rule, &rule_field_15_non_con);
  add_rule_field(&ipv6_coap_oscore_observe_notif_rule, &rule_field_16);
  add_rule_field(&ipv6_coap_oscore_observe_notif_rule, &rule_field_17_content);
  add_rule_field(&ipv6_coap_oscore_observe_notif_rule, &rule_field_18);
  add_rule_field(&ipv6_coap_oscore_observe_notif_rule, &rule_field_19);
  add_rule_field(&ipv6_coap_oscore_observe_notif_rule,
                 &rule_field_20_observe_notif);
  add_rule_field(&ipv6_coap_oscore_observe_notif_rule,
                 &rule_field_21_observe_notif);

  static rule_field_t *rule_fields_edhoc_request[21];
  static rule_t ipv6_coap_edhoc_request_rule;
  init_rule(&ipv6_coap_edhoc_request_rule, EDHOC_COAP_REQUEST_RULE_ID,
            STACK_IPV6_UDP_COAP, rule_fields_edhoc_request);
  add_ip_fields(&ipv6_coap_edhoc_request_rule);
  add_udp_fields(&ipv6_coap_edhoc_request_rule);
  add_rule_field(&ipv6_coap_edhoc_request_rule, &rule_field_14);
  add_rule_field(&ipv6_coap_edhoc_request_rule, &rule_field_15_con);
  add_rule_field(&ipv6_coap_edhoc_request_rule, &rule_field_16);
  add_rule_field(&ipv6_coap_edhoc_request_rule, &rule_field_17_post);
  add_rule_field(&ipv6_coap_edhoc_request_rule, &rule_field_18);
  add_rule_field(&ipv6_coap_edhoc_request_rule, &rule_field_19);
  add_rule_field(&ipv6_coap_edhoc_request_rule, &rule_field_edhoc_uri);

  static rule_field_t *rule_fields_edhoc_response[21];
  static rule_t ipv6_coap_edhoc_response_rule;
  init_rule(&ipv6_coap_edhoc_response_rule, EDHOC_COAP_RESPONSE_RULE_ID,
            STACK_IPV6_UDP_COAP, rule_fields_edhoc_response);

  add_ip_fields(&ipv6_coap_edhoc_response_rule);
  add_udp_fields(&ipv6_coap_edhoc_response_rule);
  add_rule_field(&ipv6_coap_edhoc_response_rule, &rule_field_14);
  add_rule_field(&ipv6_coap_edhoc_response_rule, &rule_field_15_ack);
  add_rule_field(&ipv6_coap_edhoc_response_rule, &rule_field_16);
  add_rule_field(&ipv6_coap_edhoc_response_rule, &rule_field_17_changed);
  add_rule_field(&ipv6_coap_edhoc_response_rule, &rule_field_18);
  add_rule_field(&ipv6_coap_edhoc_response_rule, &rule_field_19);

  static rule_field_t *rule_fields_6[15];
  static rule_t ipv6_udp_rule;
  init_rule(&ipv6_udp_rule, IP_UDP_RULE_ID, STACK_IPV6_UDP, rule_fields_6);

  add_ip_fields(&ipv6_udp_rule);
  add_udp_fields(&ipv6_udp_rule);

  // Initialize the rules array
  static rules_t rules;
  static rule_t *rule_array[8];
  init_rules(&rules, rule_array, NO_COMP_RULE_ID);
  add_rule(&rules, &ipv6_coap_oscore_request_rule);
  add_rule(&rules, &ipv6_coap_oscore_response_rule);
  add_rule(&rules, &ipv6_coap_oscore_observe_request_rule);
  add_rule(&rules, &ipv6_coap_oscore_observe_response_rule);
  add_rule(&rules, &ipv6_coap_oscore_observe_notif_rule);
  add_rule(&rules, &ipv6_coap_edhoc_request_rule);
  add_rule(&rules, &ipv6_coap_edhoc_response_rule);
  add_rule(&rules, &ipv6_udp_rule);

  return &rules;
}