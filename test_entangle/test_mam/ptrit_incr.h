/*
 * Copyright (c) 2018 IOTA Stiftung
 * https://github.com/iotaledger/entangled
 *
 * Refer to the LICENSE file for licensing information
 * 
 * Modifications copyright (c) Zan-Jun Wang
 */

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __COMMON_TRINARY_PTRIT_INCREMENT_H_
#define __COMMON_TRINARY_PTRIT_INCREMENT_H_

#include "ptrit.h"

void ptrit_offset(ptrit_t *const trits, size_t const length);
void ptrit_increment(ptrit_t *const trits, size_t const offset,
                     size_t const end);

#endif
#ifdef __cplusplus
}
#endif
