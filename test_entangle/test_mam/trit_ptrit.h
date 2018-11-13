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

#ifndef __COMMON_TRINARY_TRIT_PTRIT_H_
#define __COMMON_TRINARY_TRIT_PTRIT_H_

#include "stdint.h"
#include "ptrit_incr.h"
#include "trits.h"

void trits_to_ptrits(trit_t const *const trits, ptrit_t *const ptrits,
                     size_t const length);
void ptrits_to_trits(ptrit_t const *const ptrits, trit_t *const trits,
                     size_t const index, size_t length);

#endif
#ifdef __cplusplus
}
#endif
