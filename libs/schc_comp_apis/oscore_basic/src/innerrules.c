/**
 * @file innerrules.c
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
 * OSCORE Inner rules definition.
 */

#include <limits.h>
#include <stddef.h>

#include "parsercoap.h"
#include "parserudp.h"
#include "rule.h"

#define REG_COAP_URI_PATH "rd"
#define REG_COAP_CONTENT_FORMAT                                                \
  {                                                                            \
    0x28                                                                       \
  }
#define REG_COAP_URI_QUERY_1 "lwm2m=1.1"
#define REG_COAP_URI_QUERY_3 "b=U"

#define LWM2M_TLV                                                              \
  {                                                                            \
    0x2D, 0x16                                                                 \
  }

#define OMA_DEVICE "3"
#define OMA_LOCATION "6"
#define OMA_TEMPERATURE "3303"
#define OMA_LIGHT_CONTROL "3311"

#define OMA_SENSOR_VALUE "5700"
#define OMA_ON_OFF "5850"

#define OMA_INSTANCE_0 "0"

#define RESOURCE_BATTERY_LEVEL "9"

#define LOCATION_PATH_LEN 10

#define NO_COMP_RULE_ID 150

#define NB_RULES 16
#define REGISTRATION_REQ_RULE_ID 40
#define REGISTRATION_RESP_RULE_ID 41
#define REGISTRATION_CONFIRM_REQ_RULE_ID 42
#define REGISTRATION_CONFIRM_RESP_RULE_ID 43
#define TEMP_SENSOR_REQ_RULE_ID 44
#define GET_STATUS_RESP_RULE_ID 45
#define LIGHT_CONTROL_REQ_RULE_ID 46
#define LIGHT_CONTROL_RESP_RULE_ID 47
#define LIGHT_STATUS_REQ_RULE_ID 48
#define OBSERVE_LOCATION_REQUEST_RULE_ID 50
#define OBSERVE_LOCATION_RESPONSE_RULE_ID 51
#define BATTERY_LEVEL_REQ_RULE_ID 52
#define REGISTER_REFRESH_REQ_RULE_ID 53
#define RESET_RULE_ID 54
#define DEREGISTER_REQ_RULE_ID 55
#define DEREGISTER_RESP_RULE_ID 56

// Registration request.
static rule_t *get_rule_40(void)
{
  // CoAP Code.
  static uint8_t coap_code = COAP_POST;
  static target_value_t coap_code_tv = {TV_BIT_STRING, {{&coap_code, 0, 8}}};

  // CoAP Uri-Path (rd).
  static uint8_t coap_uri_path[] = REG_COAP_URI_PATH;
  static target_value_t coap_uri_path_tv = {
      TV_BIT_STRING,
      {{coap_uri_path, 0, (sizeof(coap_uri_path) - 1) * CHAR_BIT}}};

  // CoAP Content-Format.
  static uint8_t coap_content_format[] = REG_COAP_CONTENT_FORMAT;
  static target_value_t coap_content_format_tv = {
      TV_BIT_STRING,
      {{coap_content_format, 0, sizeof(coap_content_format) * CHAR_BIT}}};

  // CoAP Uri-Query 1 (lwm2m=1.1).
  static uint8_t coap_uri_query_1[] = REG_COAP_URI_QUERY_1;
  static target_value_t coap_uri_query_1_tv = {
      TV_BIT_STRING,
      {{coap_uri_query_1, 0, (sizeof(coap_uri_query_1) - 1) * CHAR_BIT}}};

  // CoAP Uri-Query 2 (ep=demoMangoh1 to demoMangoh8).
  static uint8_t mangoh1[] = "ep=demoMangoh1";
  static uint8_t mangoh2[] = "ep=demoMangoh2";
  static uint8_t mangoh3[] = "ep=demoMangoh3";
  static uint8_t mangoh4[] = "ep=demoMangoh4";
  static uint8_t mangoh5[] = "ep=demoMangoh5";
  static uint8_t mangoh6[] = "ep=demoMangoh6";
  static uint8_t mangoh7[] = "ep=demoMangoh7";
  static uint8_t mangoh8[] = "ep=demoMangoh8";

  static bit_string_t mangoh1_bs = {
      mangoh1, 0,
      (sizeof(mangoh1) - 1) /* don't count the trailing null byte*/ * CHAR_BIT};
  static bit_string_t mangoh2_bs = {mangoh2, 0,
                                    (sizeof(mangoh2) - 1) * CHAR_BIT};
  static bit_string_t mangoh3_bs = {mangoh3, 0,
                                    (sizeof(mangoh3) - 1) * CHAR_BIT};
  static bit_string_t mangoh4_bs = {mangoh4, 0,
                                    (sizeof(mangoh4) - 1) * CHAR_BIT};
  static bit_string_t mangoh5_bs = {mangoh5, 0,
                                    (sizeof(mangoh5) - 1) * CHAR_BIT};
  static bit_string_t mangoh6_bs = {mangoh6, 0,
                                    (sizeof(mangoh6) - 1) * CHAR_BIT};
  static bit_string_t mangoh7_bs = {mangoh7, 0,
                                    (sizeof(mangoh7) - 1) * CHAR_BIT};
  static bit_string_t mangoh8_bs = {mangoh8, 0,
                                    (sizeof(mangoh8) - 1) * CHAR_BIT};

  static bit_string_ptr_t coap_uri_query_2_bs_ptr[] = {
      &mangoh1_bs, &mangoh2_bs, &mangoh3_bs, &mangoh4_bs,
      &mangoh5_bs, &mangoh6_bs, &mangoh7_bs, &mangoh8_bs,
  };

  static mapped_values_t coap_uri_query_2_mv;
  coap_uri_query_2_mv.values_nb = 8;
  coap_uri_query_2_mv.values_ptr = coap_uri_query_2_bs_ptr;

  static target_value_t coap_uri_query_2_tv;
  coap_uri_query_2_tv.tv_type = TV_MAPPED_VALUES;
  coap_uri_query_2_tv.mapped_values = coap_uri_query_2_mv;

  // CoAP Uri-Query 3 (b=U).
  static uint8_t coap_uri_query_3[] = REG_COAP_URI_QUERY_3;
  static target_value_t coap_uri_query_3_tv = {
      TV_BIT_STRING,
      {{coap_uri_query_3, 0, (sizeof(coap_uri_query_3) - 1) * CHAR_BIT}}};

  // CoAP Uri-Query 4 (lt=)
  static uint8_t lt1[] = "lt=300";
  static uint8_t lt2[] = "lt=500";
  static uint8_t lt3[] = "lt=1000";
  static uint8_t lt4[] = "lt=2000";
  static uint8_t lt5[] = "lt=3600";
  static uint8_t lt6[] = "lt=7200";
  static uint8_t lt7[] = "lt=10000";
  static uint8_t lt8[] = "lt=86400";

  static bit_string_t lt1_bs = {lt1, 0, (sizeof(lt1) - 1) * CHAR_BIT};
  static bit_string_t lt2_bs = {lt2, 0, (sizeof(lt2) - 1) * CHAR_BIT};
  static bit_string_t lt3_bs = {lt3, 0, (sizeof(lt3) - 1) * CHAR_BIT};
  static bit_string_t lt4_bs = {lt4, 0, (sizeof(lt4) - 1) * CHAR_BIT};
  static bit_string_t lt5_bs = {lt5, 0, (sizeof(lt5) - 1) * CHAR_BIT};
  static bit_string_t lt6_bs = {lt6, 0, (sizeof(lt6) - 1) * CHAR_BIT};
  static bit_string_t lt7_bs = {lt7, 0, (sizeof(lt7) - 1) * CHAR_BIT};
  static bit_string_t lt8_bs = {lt8, 0, (sizeof(lt8) - 1) * CHAR_BIT};

  static bit_string_ptr_t coap_uri_query_4_bs_ptr[] = {
      &lt1_bs, &lt2_bs, &lt3_bs, &lt4_bs, &lt5_bs, &lt6_bs, &lt7_bs, &lt8_bs,
  };

  static mapped_values_t coap_uri_query_4_mv;
  coap_uri_query_4_mv.values_nb = 8;
  coap_uri_query_4_mv.values_ptr = coap_uri_query_4_bs_ptr;

  static target_value_t coap_uri_query_4_tv;
  coap_uri_query_4_tv.tv_type = TV_MAPPED_VALUES;
  coap_uri_query_4_tv.mapped_values = coap_uri_query_4_mv;

  // Rule entries.
  static rule_field_t rule_field_0 = {
      FID_COAP_CODE, 1, DIR_BI, &coap_code_tv, 8, MO_EQUAL, {0}, CDA_NOT_SENT};

  static rule_field_t rule_field_1 = {
      FID_COAP_URI_PATH, 1,   DIR_BI,      &coap_uri_path_tv, 0,
      MO_EQUAL,          {0}, CDA_NOT_SENT};

  static rule_field_t rule_field_2 = {FID_COAP_CONTENT_FORMAT,
                                      1,
                                      DIR_BI,
                                      &coap_content_format_tv,
                                      0,
                                      MO_EQUAL,
                                      {0},
                                      CDA_NOT_SENT};

  static rule_field_t rule_field_3 = {
      FID_COAP_URI_QUERY, 1,   DIR_BI,      &coap_uri_query_1_tv, 0,
      MO_EQUAL,           {0}, CDA_NOT_SENT};

  static rule_field_t rule_field_4 = {
      FID_COAP_URI_QUERY, 2,   DIR_BI,          &coap_uri_query_2_tv, 0,
      MO_MAPPING,         {0}, CDA_MAPPING_SENT};
  static rule_field_t rule_field_5 = {
      FID_COAP_URI_QUERY, 3,   DIR_BI,      &coap_uri_query_3_tv, 0,
      MO_EQUAL,           {0}, CDA_NOT_SENT};
  static rule_field_t rule_field_6 = {
      FID_COAP_URI_QUERY, 4,   DIR_BI,          &coap_uri_query_4_tv, 0,
      MO_MAPPING,         {0}, CDA_MAPPING_SENT};

  static rule_field_t *rule_fields[7];
  static rule_t rule;
  init_rule(&rule, REGISTRATION_REQ_RULE_ID, STACK_OSCORE_PLAINTEXT,
            rule_fields);
  add_rule_field(&rule, &rule_field_0);
  add_rule_field(&rule, &rule_field_1);
  add_rule_field(&rule, &rule_field_2);
  add_rule_field(&rule, &rule_field_3);
  add_rule_field(&rule, &rule_field_4);
  add_rule_field(&rule, &rule_field_5);
  add_rule_field(&rule, &rule_field_6);

  return &rule;
}

// Registration response.
static rule_t *get_rule_41(void)
{
  // CoAP Code.
  static uint8_t coap_code = COAP_CREATED;
  static target_value_t coap_code_tv = {TV_BIT_STRING, {{&coap_code, 0, 8}}};

  // CoAP Location-Path (rd).
  static uint8_t coap_uri_path[] = REG_COAP_URI_PATH;
  static target_value_t coap_uri_path_tv = {
      TV_BIT_STRING,
      {{coap_uri_path, 0, (sizeof(coap_uri_path) - 1) * CHAR_BIT}}};

  // Rule entries.
  static rule_field_t rule_field_0 = {
      FID_COAP_CODE, 1, DIR_BI, &coap_code_tv, 8, MO_EQUAL, {0}, CDA_NOT_SENT};

  static rule_field_t rule_field_1 = {FID_COAP_LOCATION_PATH,
                                      1,
                                      DIR_BI,
                                      &coap_uri_path_tv,
                                      0,
                                      MO_EQUAL,
                                      {0},
                                      CDA_NOT_SENT};

  static rule_field_t rule_field_2 = {
      FID_COAP_LOCATION_PATH,       2,         DIR_BI, NULL,
      LOCATION_PATH_LEN * CHAR_BIT, MO_IGNORE, {0},    CDA_VALUE_SENT};

  static rule_field_t *rule_fields[3];
  static rule_t rule;
  init_rule(&rule, REGISTRATION_RESP_RULE_ID, STACK_OSCORE_PLAINTEXT,
            rule_fields);
  add_rule_field(&rule, &rule_field_0);
  add_rule_field(&rule, &rule_field_1);
  add_rule_field(&rule, &rule_field_2);

  return &rule;
}

// Registration confirm request.
static rule_t *get_rule_42(void)
{
  // CoAP Code.
  static uint8_t coap_code = COAP_POST;
  static target_value_t coap_code_tv = {TV_BIT_STRING, {{&coap_code, 0, 8}}};

  // CoAP Location-Path (rd).
  static uint8_t coap_uri_path[] = REG_COAP_URI_PATH;
  static target_value_t coap_uri_path_tv = {
      TV_BIT_STRING,
      {{coap_uri_path, 0, (sizeof(coap_uri_path) - 1) * CHAR_BIT}}};

  // Rule entries.
  static rule_field_t rule_field_0 = {
      FID_COAP_CODE, 1, DIR_BI, &coap_code_tv, 8, MO_EQUAL, {0}, CDA_NOT_SENT};

  static rule_field_t rule_field_1 = {
      FID_COAP_URI_PATH, 1,   DIR_BI,      &coap_uri_path_tv, 0,
      MO_EQUAL,          {0}, CDA_NOT_SENT};

  static rule_field_t rule_field_2 = {
      FID_COAP_URI_PATH, 2,   DIR_BI,        NULL, LOCATION_PATH_LEN * CHAR_BIT,
      MO_IGNORE,         {0}, CDA_VALUE_SENT};

  static rule_field_t *rule_fields[3];
  static rule_t rule;
  init_rule(&rule, REGISTRATION_CONFIRM_REQ_RULE_ID, STACK_OSCORE_PLAINTEXT,
            rule_fields);
  add_rule_field(&rule, &rule_field_0);
  add_rule_field(&rule, &rule_field_1);
  add_rule_field(&rule, &rule_field_2);

  return &rule;
}

// Registration confirm response.
static rule_t *get_rule_43(void)
{
  // CoAP Code.
  static uint8_t coap_code = COAP_CHANGED;
  static target_value_t coap_code_tv = {TV_BIT_STRING, {{&coap_code, 0, 8}}};

  // Rule entries.
  static rule_field_t rule_field_0 = {
      FID_COAP_CODE, 1, DIR_BI, &coap_code_tv, 8, MO_EQUAL, {0}, CDA_NOT_SENT};

  static rule_field_t *rule_fields[1];
  static rule_t rule;
  init_rule(&rule, REGISTRATION_CONFIRM_RESP_RULE_ID, STACK_OSCORE_PLAINTEXT,
            rule_fields);
  add_rule_field(&rule, &rule_field_0);

  return &rule;
}

// Temperature sensor request.
static rule_t *get_rule_44(void)
{
  // CoAP Code.
  static uint8_t coap_code = COAP_GET;
  static target_value_t coap_code_tv = {TV_BIT_STRING, {{&coap_code, 0, 8}}};

  // CoAP Uri-Path (3303).
  static uint8_t coap_uri_path_1[] = OMA_TEMPERATURE;
  static target_value_t coap_uri_path_1_tv = {
      TV_BIT_STRING,
      {{coap_uri_path_1, 0, (sizeof(coap_uri_path_1) - 1) * CHAR_BIT}}};

  // CoAP Uri-Path (0).
  static uint8_t coap_uri_path_2[] = OMA_INSTANCE_0;
  static target_value_t coap_uri_path_2_tv = {
      TV_BIT_STRING,
      {{coap_uri_path_2, 0, (sizeof(coap_uri_path_2) - 1) * CHAR_BIT}}};

  // CoAP Uri-Path (5700).
  static uint8_t coap_uri_path_3[] = OMA_SENSOR_VALUE;
  static target_value_t coap_uri_path_3_tv = {
      TV_BIT_STRING,
      {{coap_uri_path_3, 0, (sizeof(coap_uri_path_3) - 1) * CHAR_BIT}}};

  // CoAP Accept (0x2d16).
  static uint8_t coap_accept[] = LWM2M_TLV;
  static target_value_t coap_accept_tv = {
      TV_BIT_STRING, {{coap_accept, 0, sizeof(coap_accept) * CHAR_BIT}}};

  // Rule entries.
  static rule_field_t rule_field_0 = {
      FID_COAP_CODE, 1, DIR_BI, &coap_code_tv, 8, MO_EQUAL, {0}, CDA_NOT_SENT};

  static rule_field_t rule_field_1 = {
      FID_COAP_URI_PATH, 1,   DIR_BI,      &coap_uri_path_1_tv, 0,
      MO_EQUAL,          {0}, CDA_NOT_SENT};

  static rule_field_t rule_field_2 = {
      FID_COAP_URI_PATH, 2,   DIR_BI,      &coap_uri_path_2_tv, 0,
      MO_EQUAL,          {0}, CDA_NOT_SENT};

  static rule_field_t rule_field_3 = {
      FID_COAP_URI_PATH, 3,   DIR_BI,      &coap_uri_path_3_tv, 0,
      MO_EQUAL,          {0}, CDA_NOT_SENT};

  static rule_field_t rule_field_4 = {
      FID_COAP_ACCEPT, 1,   DIR_BI,      &coap_accept_tv, 0,
      MO_EQUAL,        {0}, CDA_NOT_SENT};

  static rule_field_t *rule_fields[5];
  static rule_t rule;
  init_rule(&rule, TEMP_SENSOR_REQ_RULE_ID, STACK_OSCORE_PLAINTEXT,
            rule_fields);
  add_rule_field(&rule, &rule_field_0);
  add_rule_field(&rule, &rule_field_1);
  add_rule_field(&rule, &rule_field_2);
  add_rule_field(&rule, &rule_field_3);
  add_rule_field(&rule, &rule_field_4);

  return &rule;
}

// Rule for:
// - Temperature status response
// - Light status response
// - Battery status response
static rule_t *get_rule_45(void)
{
  // CoAP Code.
  static uint8_t coap_code = COAP_CONTENT;
  static target_value_t coap_code_tv = {TV_BIT_STRING, {{&coap_code, 0, 8}}};

  // CoAP Content-Format (0x2d16).
  static uint8_t coap_content_format[] = LWM2M_TLV;
  static target_value_t coap_content_format_tv = {
      TV_BIT_STRING,
      {{coap_content_format, 0, sizeof(coap_content_format) * CHAR_BIT}}};

  // Rule entries.
  static rule_field_t rule_field_0 = {
      FID_COAP_CODE, 1, DIR_BI, &coap_code_tv, 8, MO_EQUAL, {0}, CDA_NOT_SENT};

  static rule_field_t rule_field_1 = {FID_COAP_CONTENT_FORMAT,
                                      1,
                                      DIR_BI,
                                      &coap_content_format_tv,
                                      0,
                                      MO_EQUAL,
                                      {0},
                                      CDA_NOT_SENT};

  static rule_field_t *rule_fields[2];
  static rule_t rule;
  init_rule(&rule, GET_STATUS_RESP_RULE_ID, STACK_OSCORE_PLAINTEXT,
            rule_fields);
  add_rule_field(&rule, &rule_field_0);
  add_rule_field(&rule, &rule_field_1);

  return &rule;
}

// Light control request.
static rule_t *get_rule_46(void)
{
  // CoAP Code.
  static uint8_t coap_code = COAP_PUT;
  static target_value_t coap_code_tv = {TV_BIT_STRING, {{&coap_code, 0, 8}}};

  // CoAP Uri-Path (3311).
  static uint8_t coap_uri_path_1[] = OMA_LIGHT_CONTROL;
  static target_value_t coap_uri_path_1_tv = {
      TV_BIT_STRING,
      {{coap_uri_path_1, 0, (sizeof(coap_uri_path_1) - 1) * CHAR_BIT}}};

  // CoAP Uri-Path (0).
  static uint8_t coap_uri_path_2[] = OMA_INSTANCE_0;
  static target_value_t coap_uri_path_2_tv = {
      TV_BIT_STRING,
      {{coap_uri_path_2, 0, (sizeof(coap_uri_path_2) - 1) * CHAR_BIT}}};

  // CoAP Uri-Path (5850).
  static uint8_t coap_uri_path_3[] = OMA_ON_OFF;
  static target_value_t coap_uri_path_3_tv = {
      TV_BIT_STRING,
      {{coap_uri_path_3, 0, (sizeof(coap_uri_path_3) - 1) * CHAR_BIT}}};

  // CoAP Content-Format (0x2d16).
  static uint8_t coap_content_format[] = LWM2M_TLV;
  static target_value_t coap_content_format_tv = {
      TV_BIT_STRING,
      {{coap_content_format, 0, sizeof(coap_content_format) * CHAR_BIT}}};

  // Rule entries.
  static rule_field_t rule_field_0 = {
      FID_COAP_CODE, 1, DIR_BI, &coap_code_tv, 8, MO_EQUAL, {0}, CDA_NOT_SENT};

  static rule_field_t rule_field_1 = {
      FID_COAP_URI_PATH, 1,   DIR_BI,      &coap_uri_path_1_tv, 0,
      MO_EQUAL,          {0}, CDA_NOT_SENT};

  static rule_field_t rule_field_2 = {
      FID_COAP_URI_PATH, 2,   DIR_BI,      &coap_uri_path_2_tv, 0,
      MO_EQUAL,          {0}, CDA_NOT_SENT};

  static rule_field_t rule_field_3 = {
      FID_COAP_URI_PATH, 3,   DIR_BI,      &coap_uri_path_3_tv, 0,
      MO_EQUAL,          {0}, CDA_NOT_SENT};

  static rule_field_t rule_field_4 = {FID_COAP_CONTENT_FORMAT,
                                      1,
                                      DIR_BI,
                                      &coap_content_format_tv,
                                      0,
                                      MO_EQUAL,
                                      {0},
                                      CDA_NOT_SENT};

  static rule_field_t *rule_fields[5];
  static rule_t rule;
  init_rule(&rule, LIGHT_CONTROL_REQ_RULE_ID, STACK_OSCORE_PLAINTEXT,
            rule_fields);
  add_rule_field(&rule, &rule_field_0);
  add_rule_field(&rule, &rule_field_1);
  add_rule_field(&rule, &rule_field_2);
  add_rule_field(&rule, &rule_field_3);
  add_rule_field(&rule, &rule_field_4);

  return &rule;
}

// Light control response.
static rule_t *get_rule_47(void)
{
  // CoAP Code.
  static uint8_t coap_code = COAP_CHANGED;
  static target_value_t coap_code_tv = {TV_BIT_STRING, {{&coap_code, 0, 8}}};

  // Rule entries.
  static rule_field_t rule_field_0 = {
      FID_COAP_CODE, 1, DIR_BI, &coap_code_tv, 8, MO_EQUAL, {0}, CDA_NOT_SENT};

  static rule_field_t *rule_fields[1];
  static rule_t rule;
  init_rule(&rule, LIGHT_CONTROL_RESP_RULE_ID, STACK_OSCORE_PLAINTEXT,
            rule_fields);
  add_rule_field(&rule, &rule_field_0);

  return &rule;
}

// Light status request.
static rule_t *get_rule_48(void)
{
  // CoAP Code.
  static uint8_t coap_code = COAP_GET;
  static target_value_t coap_code_tv = {TV_BIT_STRING, {{&coap_code, 0, 8}}};

  // CoAP Uri-Path (3311).
  static uint8_t coap_uri_path_1[] = OMA_LIGHT_CONTROL;
  static target_value_t coap_uri_path_1_tv = {
      TV_BIT_STRING,
      {{coap_uri_path_1, 0, (sizeof(coap_uri_path_1) - 1) * CHAR_BIT}}};

  // CoAP Uri-Path (0).
  static uint8_t coap_uri_path_2[] = OMA_INSTANCE_0;
  static target_value_t coap_uri_path_2_tv = {
      TV_BIT_STRING,
      {{coap_uri_path_2, 0, (sizeof(coap_uri_path_2) - 1) * CHAR_BIT}}};

  // CoAP Uri-Path (5850).
  static uint8_t coap_uri_path_3[] = OMA_ON_OFF;
  static target_value_t coap_uri_path_3_tv = {
      TV_BIT_STRING,
      {{coap_uri_path_3, 0, (sizeof(coap_uri_path_3) - 1) * CHAR_BIT}}};

  // CoAP Accept (0x2d16).
  static uint8_t coap_accept[] = LWM2M_TLV;
  static target_value_t coap_accept_tv = {
      TV_BIT_STRING, {{coap_accept, 0, sizeof(coap_accept) * CHAR_BIT}}};

  // Rule entries.
  static rule_field_t rule_field_0 = {
      FID_COAP_CODE, 1, DIR_BI, &coap_code_tv, 8, MO_EQUAL, {0}, CDA_NOT_SENT};

  static rule_field_t rule_field_1 = {
      FID_COAP_URI_PATH, 1,   DIR_BI,      &coap_uri_path_1_tv, 0,
      MO_EQUAL,          {0}, CDA_NOT_SENT};

  static rule_field_t rule_field_2 = {
      FID_COAP_URI_PATH, 2,   DIR_BI,      &coap_uri_path_2_tv, 0,
      MO_EQUAL,          {0}, CDA_NOT_SENT};

  static rule_field_t rule_field_3 = {
      FID_COAP_URI_PATH, 3,   DIR_BI,      &coap_uri_path_3_tv, 0,
      MO_EQUAL,          {0}, CDA_NOT_SENT};

  static rule_field_t rule_field_4 = {
      FID_COAP_ACCEPT, 1,   DIR_BI,      &coap_accept_tv, 0,
      MO_EQUAL,        {0}, CDA_NOT_SENT};

  static rule_field_t *rule_fields[5];
  static rule_t rule;
  init_rule(&rule, LIGHT_STATUS_REQ_RULE_ID, STACK_OSCORE_PLAINTEXT,
            rule_fields);
  add_rule_field(&rule, &rule_field_0);
  add_rule_field(&rule, &rule_field_1);
  add_rule_field(&rule, &rule_field_2);
  add_rule_field(&rule, &rule_field_3);
  add_rule_field(&rule, &rule_field_4);

  return &rule;
}

// Observe location request
static rule_t *get_rule_50(void)
{
  // CoAP Code.
  static uint8_t coap_code = COAP_GET;
  static target_value_t coap_code_tv = {TV_BIT_STRING, {{&coap_code, 0, 8}}};

  // Observe option
  static uint8_t empty_option = 0x00;
  static target_value_t empty_option_tv = {TV_BIT_STRING,
                                           {{&empty_option, 0, 0}}};

  static uint8_t coap_uri_path_1[] = OMA_LOCATION;
  static target_value_t coap_uri_path_1_tv = {
      TV_BIT_STRING,
      {{coap_uri_path_1, 0, (sizeof(coap_uri_path_1) - 1) * CHAR_BIT}}};

  static uint8_t coap_uri_path_2[] = OMA_INSTANCE_0;
  static target_value_t coap_uri_path_2_tv = {
      TV_BIT_STRING,
      {{coap_uri_path_2, 0, (sizeof(coap_uri_path_2) - 1) * CHAR_BIT}}};

  // CoAP Accept (0x2d16).
  static uint8_t coap_accept[] = LWM2M_TLV;
  static target_value_t coap_accept_tv = {TV_BIT_STRING,
                                          {{coap_accept, 0, 16}}};

  // Rule entries.
  static rule_field_t rule_field_0 = {
      FID_COAP_CODE, 1, DIR_BI, &coap_code_tv, 8, MO_EQUAL, {0}, CDA_NOT_SENT};

  static rule_field_t rule_field_1 = {
      FID_COAP_OBSERVE, 1,   DIR_BI,      &empty_option_tv, 0,
      MO_EQUAL,         {0}, CDA_NOT_SENT};

  static rule_field_t rule_field_2 = {
      FID_COAP_URI_PATH, 1,   DIR_BI,      &coap_uri_path_1_tv, 8,
      MO_EQUAL,          {0}, CDA_NOT_SENT};

  static rule_field_t rule_field_3 = {
      FID_COAP_URI_PATH, 2,   DIR_BI,      &coap_uri_path_2_tv, 8,
      MO_EQUAL,          {0}, CDA_NOT_SENT};

  static rule_field_t rule_field_4 = {
      FID_COAP_ACCEPT, 1,   DIR_BI,      &coap_accept_tv, 16,
      MO_EQUAL,        {0}, CDA_NOT_SENT};

  static rule_field_t *rule_fields[6];
  static rule_t rule;
  init_rule(&rule, OBSERVE_LOCATION_REQUEST_RULE_ID, STACK_OSCORE_PLAINTEXT,
            rule_fields);
  add_rule_field(&rule, &rule_field_0);
  add_rule_field(&rule, &rule_field_1);
  add_rule_field(&rule, &rule_field_2);
  add_rule_field(&rule, &rule_field_3);
  add_rule_field(&rule, &rule_field_4);

  return &rule;
}

// Observe location response and notification
static rule_t *get_rule_51(void)
{
  // CoAP Code.
  static uint8_t coap_code = COAP_CONTENT;
  static target_value_t coap_code_tv = {TV_BIT_STRING, {{&coap_code, 0, 8}}};

  // Observe option
  static uint8_t empty_option = 0x00;
  static target_value_t empty_option_tv = {TV_BIT_STRING,
                                           {{&empty_option, 0, 0}}};

  // CoAP Content-Format (0x2d16).
  static uint8_t coap_content_format[] = LWM2M_TLV;
  static target_value_t coap_content_format_tv = {
      TV_BIT_STRING,
      {{coap_content_format, 0, sizeof(coap_content_format) * CHAR_BIT}}};

  // Rule entries.
  static rule_field_t rule_field_0 = {
      FID_COAP_CODE, 1, DIR_BI, &coap_code_tv, 8, MO_EQUAL, {0}, CDA_NOT_SENT};

  static rule_field_t rule_field_1 = {
      FID_COAP_OBSERVE, 1,   DIR_BI,      &empty_option_tv, 0,
      MO_EQUAL,         {0}, CDA_NOT_SENT};

  static rule_field_t rule_field_2 = {FID_COAP_CONTENT_FORMAT,
                                      1,
                                      DIR_BI,
                                      &coap_content_format_tv,
                                      0,
                                      MO_EQUAL,
                                      {0},
                                      CDA_NOT_SENT};

  static rule_field_t *rule_fields[3];
  static rule_t rule;
  init_rule(&rule, OBSERVE_LOCATION_RESPONSE_RULE_ID, STACK_OSCORE_PLAINTEXT,
            rule_fields);
  add_rule_field(&rule, &rule_field_0);
  add_rule_field(&rule, &rule_field_1);
  add_rule_field(&rule, &rule_field_2);

  return &rule;
}

// Battery level request.
static rule_t *get_rule_52(void)
{
  // CoAP Code.
  static uint8_t coap_code = COAP_GET;
  static target_value_t coap_code_tv = {TV_BIT_STRING, {{&coap_code, 0, 8}}};

  // CoAP Uri-Path (3).
  static uint8_t coap_uri_path_1[] = OMA_DEVICE;
  static target_value_t coap_uri_path_1_tv = {
      TV_BIT_STRING,
      {{coap_uri_path_1, 0, (sizeof(coap_uri_path_1) - 1) * CHAR_BIT}}};

  // CoAP Uri-Path (0).
  static uint8_t coap_uri_path_2[] = OMA_INSTANCE_0;
  static target_value_t coap_uri_path_2_tv = {
      TV_BIT_STRING,
      {{coap_uri_path_2, 0, (sizeof(coap_uri_path_2) - 1) * CHAR_BIT}}};

  // CoAP Uri-Path (9).
  static uint8_t coap_uri_path_3[] = RESOURCE_BATTERY_LEVEL;
  static target_value_t coap_uri_path_3_tv = {
      TV_BIT_STRING,
      {{coap_uri_path_3, 0, (sizeof(coap_uri_path_3) - 1) * CHAR_BIT}}};

  // CoAP Accept (0x2d16).
  static uint8_t coap_accept[] = LWM2M_TLV;
  static target_value_t coap_accept_tv = {
      TV_BIT_STRING, {{coap_accept, 0, sizeof(coap_accept) * CHAR_BIT}}};

  // Rule entries.
  static rule_field_t rule_field_0 = {
      FID_COAP_CODE, 1, DIR_BI, &coap_code_tv, 8, MO_EQUAL, {0}, CDA_NOT_SENT};

  static rule_field_t rule_field_1 = {
      FID_COAP_URI_PATH, 1,   DIR_BI,      &coap_uri_path_1_tv, 0,
      MO_EQUAL,          {0}, CDA_NOT_SENT};

  static rule_field_t rule_field_2 = {
      FID_COAP_URI_PATH, 2,   DIR_BI,      &coap_uri_path_2_tv, 0,
      MO_EQUAL,          {0}, CDA_NOT_SENT};

  static rule_field_t rule_field_3 = {
      FID_COAP_URI_PATH, 3,   DIR_BI,      &coap_uri_path_3_tv, 0,
      MO_EQUAL,          {0}, CDA_NOT_SENT};

  static rule_field_t rule_field_4 = {
      FID_COAP_ACCEPT, 1,   DIR_BI,      &coap_accept_tv, 0,
      MO_EQUAL,        {0}, CDA_NOT_SENT};

  static rule_field_t *rule_fields[5];
  static rule_t rule;
  init_rule(&rule, BATTERY_LEVEL_REQ_RULE_ID, STACK_OSCORE_PLAINTEXT,
            rule_fields);
  add_rule_field(&rule, &rule_field_0);
  add_rule_field(&rule, &rule_field_1);
  add_rule_field(&rule, &rule_field_2);
  add_rule_field(&rule, &rule_field_3);
  add_rule_field(&rule, &rule_field_4);

  return &rule;
}

// Register refresh request
static rule_t *get_rule_53(void)
{
  // CoAP code
  static uint8_t coap_code = COAP_POST;
  static target_value_t coap_code_tv = {TV_BIT_STRING, {{&coap_code, 0, 8}}};

  // CoAP Uri-Path ("rd")
  static uint8_t coap_uri_path_1[] = REG_COAP_URI_PATH;
  static target_value_t coap_uri_path_1_tv = {
      TV_BIT_STRING,
      {{coap_uri_path_1, 0, (sizeof(coap_uri_path_1) - 1) * CHAR_BIT}}};

  // Rule entries
  static rule_field_t rule_field_0 = {
      FID_COAP_CODE, 1, DIR_BI, &coap_code_tv, 0, MO_EQUAL, {0}, CDA_NOT_SENT};

  static rule_field_t rule_field_1 = {
      FID_COAP_URI_PATH, 1,   DIR_BI,      &coap_uri_path_1_tv, 0,
      MO_EQUAL,          {0}, CDA_NOT_SENT};
  static rule_field_t rule_field_2 = {
      FID_COAP_URI_PATH, 2,   DIR_BI,        NULL, LOCATION_PATH_LEN * CHAR_BIT,
      MO_IGNORE,         {0}, CDA_VALUE_SENT};

  static rule_field_t *rule_fields[3];
  static rule_t rule;
  init_rule(&rule, REGISTER_REFRESH_REQ_RULE_ID, STACK_OSCORE_PLAINTEXT,
            rule_fields);
  add_rule_field(&rule, &rule_field_0);
  add_rule_field(&rule, &rule_field_1);
  add_rule_field(&rule, &rule_field_2);

  return &rule;
}

// Reset message
static rule_t *get_rule_54(void)
{
  // CoAP code
  static uint8_t coap_code = COAP_EMPTY;
  static target_value_t coap_code_tv = {TV_BIT_STRING, {{&coap_code, 0, 8}}};

  // Rule entry
  static rule_field_t rule_field_0 = {
      FID_COAP_CODE, 1, DIR_BI, &coap_code_tv, 0, MO_EQUAL, {0}, CDA_NOT_SENT};

  static rule_field_t *rule_fields[1];
  static rule_t rule;
  init_rule(&rule, RESET_RULE_ID, STACK_OSCORE_PLAINTEXT, rule_fields);
  add_rule_field(&rule, &rule_field_0);

  return &rule;
}

// Deregister request
static rule_t *get_rule_55(void)
{
  // CoAP code
  static uint8_t coap_code = COAP_DELETE;
  static target_value_t coap_code_tv = {TV_BIT_STRING, {{&coap_code, 0, 8}}};

  // CoAP Uri-Path ("rd")
  static uint8_t coap_uri_path_1[] = REG_COAP_URI_PATH;
  static target_value_t coap_uri_path_1_tv = {
      TV_BIT_STRING,
      {{coap_uri_path_1, 0, (sizeof(coap_uri_path_1) - 1) * CHAR_BIT}}};

  // Rule entries
  static rule_field_t rule_field_0 = {
      FID_COAP_CODE, 1, DIR_BI, &coap_code_tv, 0, MO_EQUAL, {0}, CDA_NOT_SENT};

  static rule_field_t rule_field_1 = {
      FID_COAP_URI_PATH, 1,   DIR_BI,      &coap_uri_path_1_tv, 0,
      MO_EQUAL,          {0}, CDA_NOT_SENT};
  static rule_field_t rule_field_2 = {
      FID_COAP_URI_PATH, 2,   DIR_BI,        NULL, LOCATION_PATH_LEN * CHAR_BIT,
      MO_IGNORE,         {0}, CDA_VALUE_SENT};

  static rule_field_t *rule_fields[3];
  static rule_t rule;
  init_rule(&rule, DEREGISTER_REQ_RULE_ID, STACK_OSCORE_PLAINTEXT, rule_fields);
  add_rule_field(&rule, &rule_field_0);
  add_rule_field(&rule, &rule_field_1);
  add_rule_field(&rule, &rule_field_2);

  return &rule;
}

// Deregister response
static rule_t *get_rule_56(void)
{
  // CoAP code
  static uint8_t coap_code = COAP_DELETED;
  static target_value_t coap_code_tv = {TV_BIT_STRING, {{&coap_code, 0, 8}}};

  // Rule entry
  static rule_field_t rule_field_0 = {
      FID_COAP_CODE, 1, DIR_BI, &coap_code_tv, 0, MO_EQUAL, {0}, CDA_NOT_SENT};

  static rule_field_t *rule_fields[1];
  static rule_t rule;
  init_rule(&rule, DEREGISTER_RESP_RULE_ID, STACK_OSCORE_PLAINTEXT,
            rule_fields);
  add_rule_field(&rule, &rule_field_0);

  return &rule;
}

rules_t *get_orange_labs_inner_rules(void)
{
  // Initialize the rules array.
  static rules_t rules;
  static rule_t *rule_array[NB_RULES];
  init_rules(&rules, rule_array, NO_COMP_RULE_ID);
  add_rule(&rules, get_rule_40());
  add_rule(&rules, get_rule_41());
  add_rule(&rules, get_rule_42());
  add_rule(&rules, get_rule_43());
  add_rule(&rules, get_rule_44());
  add_rule(&rules, get_rule_45());
  add_rule(&rules, get_rule_46());
  add_rule(&rules, get_rule_47());
  add_rule(&rules, get_rule_48());
  add_rule(&rules, get_rule_50());
  add_rule(&rules, get_rule_51());
  add_rule(&rules, get_rule_52());
  add_rule(&rules, get_rule_53());
  add_rule(&rules, get_rule_54());
  add_rule(&rules, get_rule_55());
  add_rule(&rules, get_rule_56());

  return &rules;
}
