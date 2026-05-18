
#include <limits.h>
#include <string.h>

#include "rule.h"
#include "schccomp.h"
#include "template.h"
#include "templatecommon.h"
#include "parsercoap.h"

#define NB_RULES 1
#define NO_COMP_RULE_ID 150
#define IPV6_UDP_RULE_ID 28

// TODO(flavien): value arbitrary choosen. It should match with IPCore one.
#define TEMPLATE_ID 25

// Number of template parameters.
#define NB_PARAMS 6

// IPv6 parameters.
static uint8_t dev_ip[IPV6_ADDRESS_LENGTH_BYTES];
static bit_string_t dev_ip_prefix_bs = {dev_ip, 0,
                                        IPV6_PREFIX_SIZE_BYTES *CHAR_BIT};
static bit_string_t *dev_ip_prefix_bs_ptr[1] = {&dev_ip_prefix_bs};
static bit_string_t dev_ip_iid_bs = {&dev_ip[IPV6_PREFIX_SIZE_BYTES], 0,
                                     IPV6_IID_SIZE_BYTES *CHAR_BIT};
static bit_string_t *dev_ip_iid_bs_ptr[1] = {&dev_ip_iid_bs};

static uint8_t app_ip[IPV6_ADDRESS_LENGTH_BYTES];
static bit_string_t app_ip_prefix_bs = {app_ip, 0,
                                        IPV6_PREFIX_SIZE_BYTES *CHAR_BIT};
static bit_string_t *app_ip_prefix_bs_ptr[1] = {&app_ip_prefix_bs};
static bit_string_t app_ip_iid_bs = {&app_ip[IPV6_PREFIX_SIZE_BYTES], 0,
                                     IPV6_IID_SIZE_BYTES *CHAR_BIT};
static bit_string_t *app_ip_iid_bs_ptr[1] = {&app_ip_iid_bs};

// UDP parameters.
static uint8_t dev_port[IP_PORT_LENGTH_BYTES];
static bit_string_t dev_port_bs = {dev_port, 0, IP_PORT_LENGTH_BYTES *CHAR_BIT};
static bit_string_t *dev_port_bs_ptr[1] = {&dev_port_bs};

static uint8_t app_port[IP_PORT_LENGTH_BYTES];
static bit_string_t app_port_bs = {app_port, 0, IP_PORT_LENGTH_BYTES *CHAR_BIT};
static bit_string_t *app_port_bs_ptr[1] = {&app_port_bs};

static template_t tpl;
static template_param_t *params_ptr[NB_PARAMS];
static template_param_t params[NB_PARAMS];

template_t *tpl_init_template(void)
{
  // Initialize the template.
  init_param(&params[0], dev_ip_prefix_bs_ptr, 1, IPV6_PREFIX_SIZE_BYTES);
  init_param(&params[1], dev_ip_iid_bs_ptr, 1, IPV6_IID_SIZE_BYTES);
  init_param(&params[2], app_ip_prefix_bs_ptr, 1, IPV6_PREFIX_SIZE_BYTES);
  init_param(&params[3], app_ip_iid_bs_ptr, 1, IPV6_IID_SIZE_BYTES);
  init_param(&params[4], dev_port_bs_ptr, 1, IP_PORT_LENGTH_BYTES);
  init_param(&params[5], app_port_bs_ptr, 1, IP_PORT_LENGTH_BYTES);

  init_template(&tpl, params_ptr, TEMPLATE_ID);
  for (uint8_t i = 0; i < NB_PARAMS; i++)
  {
    add_param(&tpl, &params[i]);
  }

  return &tpl;
}

rules_t *tpl_get_template_rules(void)
{
  // IPV6 layer.
  // Version.
  static uint8_t ipv6_version = 0x60; // Only 4 MSB bits are used.
  static target_value_t ipv6_version_tv = {TV_BIT_STRING,
                                           {{&ipv6_version, 0, 4}}};
  // Traffic Class.
  static uint8_t ipv6_traffic_class = 0;
  static target_value_t ipv6_traffic_class_tv = {TV_BIT_STRING,
                                                 {{&ipv6_traffic_class, 0, 8}}};
  //  Flow Label.
  static uint8_t ipv6_flow_label[] = {0x00, 0x00,
                                      0x00}; // Only 20 MSB bits are used.
  static target_value_t ipv6_flow_label_tv = {TV_BIT_STRING,
                                              {{ipv6_flow_label, 0, 20}}};
  // Next Header for UDP.
  static uint8_t ipv6_next_header_udp = 17; // For UDP.
  static target_value_t ipv6_next_header_udp_tv = {
      TV_BIT_STRING, {{&ipv6_next_header_udp, 0, 8}}};

  // Hop Limit.
  static uint8_t ipv6_hop_limit = 255;
  static target_value_t ipv6_hop_limit_tv = {TV_BIT_STRING,
                                             {{&ipv6_hop_limit, 0, 8}}};
  // Device Prefix.
  static target_value_t ipv6_prefix_dev_tv = {TV_BIT_STRING, {{dev_ip, 0, 64}}};
  // Device IID.
  static target_value_t ipv6_iid_dev_tv = {TV_BIT_STRING,
                                           {{dev_ip + 8, 0, 64}}};
  // Remote app prefix.
  static target_value_t ipv6_prefix_app_tv = {TV_BIT_STRING, {{app_ip, 0, 64}}};
  // Remote app IID.
  static target_value_t ipv6_iid_app_tv = {TV_BIT_STRING,
                                           {{app_ip + 8, 0, 64}}};
  // UDP Device port.
  static target_value_t udp_port_dev_tv = {TV_BIT_STRING, {{dev_port, 0, 16}}};
  // UDP Application port.
  static target_value_t udp_port_app_tv = {TV_BIT_STRING, {{app_port, 0, 16}}};

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

  // IPv6 Device Prefix.
  static rule_field_t rule_field_06 = {
      FID_IPV6_PREFIX_DEV, 1, DIR_BI, &ipv6_prefix_dev_tv, 64, MO_EQUAL, {0},
      CDA_NOT_SENT};

  // IPv6 Device IID.
  static rule_field_t rule_field_07 = {
      FID_IPV6_IID_DEV, 1,   DIR_BI,      &ipv6_iid_dev_tv, 64,
      MO_EQUAL,         {0}, CDA_NOT_SENT};

  // IPv6 Application Prefix.
  static rule_field_t rule_field_08 = {
      FID_IPV6_PREFIX_APP, 1, DIR_BI, &ipv6_prefix_app_tv, 64, MO_EQUAL, {0},
      CDA_NOT_SENT};

  // IPv6 Application IID.
  static rule_field_t rule_field_09 = {
      FID_IPV6_IID_APP, 1,   DIR_BI,      &ipv6_iid_app_tv, 64,
      MO_EQUAL,         {0}, CDA_NOT_SENT};

  // Rule entry definitions for UDP layer.
  // UDP Device Port.
  static rule_field_t rule_field_10 = {
      FID_UDP_PORT_DEV, 1,   DIR_BI,      &udp_port_dev_tv, 16,
      MO_EQUAL,         {0}, CDA_NOT_SENT};

  // UDP Application Port.
  static rule_field_t rule_field_11 = {
      FID_UDP_PORT_APP, 1,   DIR_BI,      &udp_port_app_tv, 16,
      MO_EQUAL,         {0}, CDA_NOT_SENT};

  // UDP Length.
  static rule_field_t rule_field_12 = {
      FID_UDP_LENGTH, 1, DIR_BI, NULL, 16, MO_IGNORE, {0}, CDA_COMPUTE_LENGTH};

  // UDP Checksum.
  static rule_field_t rule_field_13 = {
      FID_UDP_CHECKSUM,    1, DIR_BI, NULL, 16, MO_IGNORE, {0},
      CDA_COMPUTE_CHECKSUM};

  // CoAP version.
  static uint8_t coap_version = 0x01;
  static target_value_t coap_version_tv = {TV_BIT_STRING,
                                           {{&coap_version, 6, 2}}};

  static rule_field_t rule_field_14 = {
    FID_COAP_VERSION, 1,   DIR_BI,      &coap_version_tv, 2,
    MO_EQUAL,         {0}, CDA_NOT_SENT};

  // CoAP Type.
  static uint8_t con_coap_type = 0b00;
  static target_value_t con_coap_type_tv = {TV_BIT_STRING,
                                            {{&con_coap_type, 6, 2}}};

  static rule_field_t rule_field_15_con = {
    FID_COAP_TYPE, 1,   DIR_BI,      &con_coap_type_tv, 2,
    MO_EQUAL,      {0}, CDA_NOT_SENT};

  // CoAP Token Length
  static uint8_t coap_tkl = 0;
  static target_value_t coap_tkl_tv = {TV_BIT_STRING, {{&coap_tkl, 4, 4}}};
  static rule_field_t rule_field_16 = {
    FID_COAP_TOKEN_LENGTH, 1, DIR_BI, &coap_tkl_tv, 4, MO_EQUAL, {0},
    CDA_NOT_SENT};

  // CoAP Code
  static uint8_t post_coap_code = COAP_POST;
  static target_value_t post_coap_code_tv = {TV_BIT_STRING,
                                             {{&post_coap_code, 0, 8}}};

  static rule_field_t rule_field_17_post = {
    FID_COAP_CODE, 1,   DIR_BI,      &post_coap_code_tv, 8,
    MO_EQUAL,      {0}, CDA_NOT_SENT};

  // CoAP Message ID
  static rule_field_t rule_field_18 = {
    FID_COAP_MSG_ID, 1, DIR_BI, NULL, 16, MO_IGNORE, {0}, CDA_VALUE_SENT};

  // CoAP Token
  static uint8_t token_placeholder = 0;

  static target_value_t coap_tk_tv = {TV_BIT_STRING, {{&token_placeholder, 0, 0}}};
  static rule_field_t rule_field_19 = {
    FID_COAP_TOKEN, 1, DIR_BI, &coap_tk_tv, 0, MO_EQUAL, {0},
    CDA_NOT_SENT};

  // CoAP Uri Path 1
  static const char uri_path_1[] = "notifications";
  static const int uri_path_1_bit_len = (sizeof(uri_path_1) - 1)*8;

  static target_value_t uri_path_1_tv = {
    TV_BIT_STRING, {{&uri_path_1, 0, uri_path_1_bit_len}}};

  static rule_field_t rule_field_20 = {
    FID_COAP_URI_PATH, 1,   DIR_BI,      &uri_path_1_tv, uri_path_1_bit_len,
    MO_EQUAL,      {0}, CDA_NOT_SENT};

  // CoAP Uri Path 2
  static const char uri_path_2[] = "temp";
  static const int uri_path_2_bit_len = (sizeof(uri_path_2) - 1)*8;

  static target_value_t uri_path_2_tv = {
    TV_BIT_STRING, {{&uri_path_2, 0, uri_path_2_bit_len}}};

  static rule_field_t rule_field_21 = {
    FID_COAP_URI_PATH, 2,   DIR_BI,      &uri_path_2_tv, uri_path_2_bit_len,
    MO_EQUAL,      {0}, CDA_NOT_SENT};

  // CoAP Content Format
  static uint8_t content_format = 0x3c;

  static target_value_t content_format_tv = {
    TV_BIT_STRING, {{&content_format, 0, 8}}};

  static rule_field_t rule_field_22 = {
    FID_COAP_CONTENT_FORMAT, 1,   DIR_BI,      &content_format_tv, 8,
    MO_EQUAL,      {0}, CDA_NOT_SENT};

  // Initialize IPv6/UDP rule.
  static rule_field_t *rule_fields_1[23];

  static rule_t ipv6_udp_rule;
  init_rule(&ipv6_udp_rule, IPV6_UDP_RULE_ID, STACK_IPV6_UDP_COAP, rule_fields_1);
  add_rule_field(&ipv6_udp_rule, &rule_field_00);
  add_rule_field(&ipv6_udp_rule, &rule_field_01);
  add_rule_field(&ipv6_udp_rule, &rule_field_02);
  add_rule_field(&ipv6_udp_rule, &rule_field_03);
  add_rule_field(&ipv6_udp_rule, &rule_field_04);
  add_rule_field(&ipv6_udp_rule, &rule_field_05);
  add_rule_field(&ipv6_udp_rule, &rule_field_06);
  add_rule_field(&ipv6_udp_rule, &rule_field_07);
  add_rule_field(&ipv6_udp_rule, &rule_field_08);
  add_rule_field(&ipv6_udp_rule, &rule_field_09);
  add_rule_field(&ipv6_udp_rule, &rule_field_10);
  add_rule_field(&ipv6_udp_rule, &rule_field_11);
  add_rule_field(&ipv6_udp_rule, &rule_field_12);
  add_rule_field(&ipv6_udp_rule, &rule_field_13);
  add_rule_field(&ipv6_udp_rule, &rule_field_14);
  add_rule_field(&ipv6_udp_rule, &rule_field_15_con);
  add_rule_field(&ipv6_udp_rule, &rule_field_16);
  add_rule_field(&ipv6_udp_rule, &rule_field_17_post);
  add_rule_field(&ipv6_udp_rule, &rule_field_18);
  add_rule_field(&ipv6_udp_rule, &rule_field_19);
  add_rule_field(&ipv6_udp_rule, &rule_field_20);
  add_rule_field(&ipv6_udp_rule, &rule_field_21);
  add_rule_field(&ipv6_udp_rule, &rule_field_22);

  // initialize the rules array.
  static rules_t rules;
  static rule_t *rule_array[NB_RULES];
  init_rules(&rules, rule_array, NO_COMP_RULE_ID);
  add_rule(&rules, &ipv6_udp_rule);

  return &rules;
}

rules_t *tpl_get_payload_rules(void)
{
  // NOP
  return NULL;
}

uint8_t tpl_get_template_id(void)
{
  return tpl.id;
}

tpl_status_t tpl_set_template_cert_rules(const uint8_t *const compact_rules,
                                         uint16_t compact_rules_len,
                                         uint8_t *const memory_ptr, // NOLINT
                                         uint16_t memory_len)
{
  (void)compact_rules;
  (void)compact_rules_len;
  (void)memory_ptr;
  (void)memory_len;
  // NOP

  return TPL_OPERATION_NOT_SUPPORTED;
}
tpl_status_t tpl_set_template_rules(const uint8_t *const compact_rules,
                                    uint16_t compact_rules_len,
                                    uint8_t *const memory_ptr, // NOLINT
                                    uint16_t memory_len)
{
  (void)compact_rules;
  (void)compact_rules_len;
  (void)memory_ptr;
  (void)memory_len;
  // NOP

  return TPL_OPERATION_NOT_SUPPORTED;
}

tpl_status_t tpl_set_payload_pattern_rules(const uint8_t *const compact_rules,
                                           uint16_t compact_rules_len,
                                           uint8_t *const memory_ptr, // NOLINT
                                           uint16_t memory_len)
{
  (void)compact_rules;
  (void)compact_rules_len;
  (void)memory_ptr;
  (void)memory_len;
  // NOP

  return TPL_OPERATION_NOT_SUPPORTED;
}

bool tpl_is_configured(void)
{
  return tpl_is_template_configured(&tpl);
}

bool tpl_is_provisioned(void)
{
  return true;
}

tpl_status_t tpl_set_index_param_value(uint8_t index, const uint8_t *value,
                                       uint16_t length)
{
  return tpl_set_param_value(&tpl, index, value, length);
}

tpl_status_t tpl_get_index_param_value(uint8_t index, uint8_t **value,
                                       uint16_t *length)
{
  return tpl_get_param_value(&tpl, index, value, length);
}

uint8_t tpl_get_nb_template_params(void)
{
  return NB_PARAMS;
}

void tpl_clear_template_params(void)
{
  // NOP
}

tpl_ip_version_t tpl_get_ip_version(void)
{
  return TPL_NET_IPV6;
}

tpl_status_t tpl_get_ip_dev_address(uint8_t *value)
{
  memcpy(value, dev_ip, IPV6_ADDRESS_LENGTH_BYTES);
  return TPL_SUCCESS;
}

tpl_status_t tpl_get_udp_dev_port(uint8_t *value)
{
  memcpy(value, dev_port, IP_PORT_LENGTH_BYTES);
  return TPL_SUCCESS;
}

tpl_status_t tpl_ruleparser_parse_frag_profile(uint8_t const *data,
                                               uint16_t size)
{
  (void)data;
  (void)size;
  return TPL_OPERATION_NOT_SUPPORTED;
}

tpl_status_t tpl_ruleparser_parse_qos(uint8_t const *data, uint16_t size)
{
  (void)data;
  (void)size;
  return TPL_OPERATION_NOT_SUPPORTED;
}

tpl_status_t tpl_ruleparser_parse_template(uint8_t const *data, uint16_t size,
                                           uint8_t sid_size)
{
  (void)data;
  (void)size;
  (void)sid_size;
  return TPL_OPERATION_NOT_SUPPORTED;
}