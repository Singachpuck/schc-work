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
 * Author: Thibaut Artis thibaut.artis@ackl.io
 */

#ifndef BASE64_H_
#define BASE64_H_

#include <stdlib.h>

char *b64_encode(const unsigned char *in, size_t len);
unsigned char *b64_decode(const char *in, size_t *outlen);

#endif