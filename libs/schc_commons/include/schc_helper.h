#ifndef SCHC_HELPER_H
#define SCHC_HELPER_H
#include <stdint.h>
#include <stdbool.h>

#include <fullsdknet.h>
#include <fullsdkfragapi.h>

#include "schc_sdk_params.h"
#include "net_helper.h"

extern bool mgt_process_request;

typedef struct {
#ifdef L2_STACK_udp
  char addr[IPV4_STR_MAX_LEN];
#endif

  char port[UDP_PORT_STR_MAX_LEN];
} l2_addr_t;

typedef struct {
  uint8_t* value;
  uint16_t size;
  bool initialized;
} schc_template_param_t;

typedef struct schc_config {
  uint16_t mtu;
  l2_addr_t host_addr;
  l2_addr_t dst_addr;
  net_callbacks_t net_callbacks;
  sdk_mode_t mode;
  schc_template_param_t* params;
} schc_config_t;

typedef enum schc_config_result {
  SCHC_CONFIG_OK, SCHC_CONFIG_KO
} schc_config_result_t;

schc_config_result_t start_config();

schc_config_result_t with_mtu(const uint16_t mtu);

schc_config_result_t with_sdk_mode(const sdk_mode_t mode);

schc_config_result_t with_tunnel(const char *host_addr, const char *host_port, const char *dst_addr,
                                 const char *dst_port);

schc_config_result_t with_net_callbacks(const net_callbacks_t net_callbacks);

schc_config_result_t with_ipv6_src_param(const char* dev_addr);

schc_config_result_t with_ipv6_dst_param(const char* app_addr);

schc_config_result_t with_ipv6_udp_src_param(const char* src_param);

schc_config_result_t with_ipv6_udp_dst_param(const char* dst_param);

schc_config_result_t config_schc();

#endif