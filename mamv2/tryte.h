/*
 * Copyright (c) 2018 IOTA Stiftung
 * https://github.com/iotaledger/entangled
 *
 * Refer to the LICENSE file for licensing information
 */

#ifndef __COMMON_TRINARY_TRYTE_H_
#define __COMMON_TRINARY_TRYTE_H_

#include "def_stdint.h"

typedef int8_t tryte_t;

#define INDEX_OF_TRYTE(tryte) ((tryte) == '9' ? 0 : ((tryte) - 'A' + 1))

#endif  // __COMMON_TRINARY_TRYTE_H_
