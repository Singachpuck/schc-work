/**
 * @file outerrules.h
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

#ifndef OUTERRULES_H
#define OUTERRULES_H

#include "fullsdkcdpriv.h"
#include "fullsdknet.h"
#include "schccomp.h"

extern uint8_t host_ipv6_addr[IPV6_ADDRESS_LENGTH_BYTES];
extern uint8_t host_udp_port[IP_PORT_LENGTH_BYTES];

extern uint8_t remote_ipv6_addr[IPV6_ADDRESS_LENGTH_BYTES];
extern uint8_t remote_udp_port[IP_PORT_LENGTH_BYTES];

const rules_t *get_orangelabs_outer_rules(void);

#endif // OUTERRULES_H;