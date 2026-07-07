/**
 * @file innerrules.h
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

#ifndef INNERRULES_H
#define INNERRULES_H

#include "rule.h"

/**
 * @brief Get the orange labs inner rules object
 *
 * @return rules_t* pointer to the list of Inner rules
 */
rules_t *get_orange_labs_inner_rules(void);

#endif // INNERRULES_H;