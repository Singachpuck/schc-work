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
 * L2 common structures and functions.
 */

#ifndef FULLSDK_L2_COMMON_H
#define FULLSDK_L2_COMMON_H

typedef enum
{
  L2_SUCCESS = 0,
  L2_ERROR,
  L2_DOWN_AVAIL,
  L2_TX_DONE,
  L2_TX_FAIL,
  L2_CONN_LOST,
  L2_CONN_AVAIL,
  L2_INTERN_ERR,
  L2_NOT_SUPPORTED_ERR
} l2_status_t;

typedef enum
{
  L2_NETWORK_ACTIVATION_TYPE_NONE = 0,
  L2_NETWORK_ACTIVATION_TYPE_ABP = 1,
  L2_NETWORK_ACTIVATION_TYPE_OTAA = 2,
} l2_network_activation_type_t;

typedef uint8_t l2_region_id_t;

typedef struct
{
  char *str;
  l2_region_id_t id;
} l2_region_t;
typedef enum
{
  L2_MSG_TYPE_UNCONFIRMED = 0,
  L2_MSG_TYPE_CONFIRMED = !L2_MSG_TYPE_UNCONFIRMED
} l2_msg_type_t;

typedef struct
{
  l2_msg_type_t type;
  uint8_t port;
  uint8_t size;
  uint8_t *buffer;
} l2_app_data_t;

#endif // FULLSDK_L2_COMMON_H