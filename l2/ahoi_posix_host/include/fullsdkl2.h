/**
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
 * Author: Flavien Moullec flavien@ackl.io
 *
 * x86 L2 layer interface.
 */

#ifndef FULLSDK_L2_H_
#define FULLSDK_L2_H_

#include <stdbool.h>
#include <stdint.h>

#include "fullsdkl2a.h"
#include "fullsdkl2common.h"

#define BCDMA_MAC_ADDRESS_SIZE 6

// This global variable must be defined in L2A layer and initialized
// to L2A_DEFAULT. Its use is reserved.
extern l2a_technology_t l2a_technology;

/**
 * This function sets l2a_technology global variable defined in L2A layer.
 * When used, it must be called before mgt_initialize().
 */
void l2_set_technology(l2a_technology_t technology);

/**
 * This function gets the class of the device.
 *
 * Parameters:
 * - class: device class ('A' or 'C')
 *
 * Returned value: l2_status_t
 */
l2_status_t l2_get_class(char *class);

/**
 * This function sets the class of the device.
 *
 * Parameters:
 * - class: device class ('A' or 'C')
 *
 * Returned value: l2_status_t
 */
l2_status_t l2_set_class(char class);

l2_status_t l2_set_dr(uint8_t dr);

l2_status_t l2_get_dr(uint8_t *dr);

l2_status_t l2_set_adr(bool adr);

/**
 * This function gets the region defined during the compilation time.
 *
 * Parameters:
 * - region: the region (LORAMAC_REGION_AS923, ...)
 *
 * Returned value: none
 */
void l2_get_region(int8_t *region);

/**
 * This function sets join activation process to OTAA.
 *
 * Parameters:
 * - otaa_on: true to set OTAA on (ABP off), false to set it off (ABP on)
 *
 * Returned value: true if the operation is successful, false if not
 */
bool l2_set_otaa(bool otaa_on);

/**
 * This function gets join activation process to OTAA.
 *
 * Parameters:
 * - otaa_on: true if OTAA is on (ABP off), false if it is off (ABP on)
 *
 * Returned value: none
 */
void l2_get_otaa(bool *otaa_on);

/**
 * This function sets whether or not confirmation is required.
 *
 * Parameters:
 * - conf_mode_on: true if confirmation is required, false if it is not
 *
 * Returned value: true if the operation is successful, false if not
 */
bool l2_set_conf_mode(uint8_t conf_mode_on);

/**
 * This function gets whether or not confirmation mode is on.
 *
 * Parameters:
 * - conf_mode_on: true if confirmation is on, false if it is not
 *
 * Returned value: none
 */
void l2_get_conf_mode(uint8_t *conf_mode_on);

/**
 * This function gets whether or not duty cycle restriction is enabled.
 *
 * Parameters:
 * - value: true if duty cycle restriction is enabled, false if it is not
 *
 * Returned value: l2_status_t
 */
l2_status_t l2_get_dutycycle(bool *value);

/**
 * This function sets whether or not duty cycle restriction is enabled.
 *
 * Parameters:
 * - dutycycle_on: true if duty cycle restriction is enabled, false if it is not
 *
 * Returned value: l2_status_t
 */
l2_status_t l2_set_dutycycle(bool dutycycle_on);

/**
 * @brief Tis functions sets the L2 MTU
 *
 * @param mtu L2 MTU in bytes
 */
void l2_set_mtu(uint16_t mtu);

/**
 * @brief configure IPv6 source address in the socket to be binded
 *
 * @param[in] add IPv6 source address
 */
void l2_set_ipv6_host_addr(const char *addr);

/**
 * @brief configure IPv6 destination address in the socket
 *
 * @param[in] addr IPv6 destination address
 */
void l2_set_ipv6_remote_addr(const char *addr);

/**
 * @brief configure UDP source port in the socket
 *
 * @param[in] port source port
 */
void l2_set_udp_src_port(const char *port);

/**
 * @brief configure UDP destination port in the socket
 *
 * @param[in] port destination port
 */
void l2_set_udp_dest_port(const char *port);

void l2_set_serial_port(const char* port);

void l2_set_iid(uint8_t val);

void l2_deinit();

/**
 * Mock functions
 */
const uint8_t *bcdma_get_dest_addr(void);

const uint8_t *bcdma_get_src_addr(void);

void bcdma_set_dest_addr(const uint8_t *addr);

void bcdma_set_src_addr(const uint8_t *addr);

#endif /* FULLSDK_L2_H_ */
