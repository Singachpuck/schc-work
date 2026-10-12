#ifndef FULL_SDK_EXT_API_H
#define FULL_SDK_EXT_API_H

#include <fullsdkmgt.h>

void net_set_host_static_ip(const char *ipv6_address);

void net_set_host_static_port(const char *udp_port);

void net_set_remote_static_ip(const char *ipv6_address);

void net_set_remote_static_port(const char *udp_port);

mgt_status_t mgt_ext_oscore_inner_compression(uint8_t *buf_out,
                                              uint16_t buf_out_size,
                                              uint16_t *out_data_size,
                                              const uint8_t *in_data,
                                              uint16_t in_data_size);

mgt_status_t mgt_ext_oscore_inner_decompression(uint8_t *out_buf,
                                                uint16_t out_buf_size,
                                                uint16_t *out_data_size,
                                                const uint8_t *in_data,
                                                uint16_t in_data_size);

#endif
