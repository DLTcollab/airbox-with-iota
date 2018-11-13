/*
 * Copyright (c) 2018 IOTA Stiftung
 * https://github.com/iotaledger/entangled
 *
 * Refer to the LICENSE file for licensing information
 * 
 * Modifications copyright (c) Zan-Jun Wang
 */

#include "trit_ptrit.h"

#define HIGH_BITS 0xFFFFFFFFFFFFFFFF
#define LOW_BITS 0x0000000000000000

void trits_to_ptrits(trit_t const *const trits, ptrit_t *const ptrits,
                     size_t const length) {
  int i;
  for (i = 0; i < length; i++) {
    switch (trits[i]) {
      case 0:
        ptrits[i].low = HIGH_BITS;
        ptrits[i].high = HIGH_BITS;
        break;
      case 1:
        ptrits[i].low = LOW_BITS;
        ptrits[i].high = HIGH_BITS;
        break;
      default:
        ptrits[i].low = HIGH_BITS;
        ptrits[i].high = LOW_BITS;
    }
  }
}

void ptrits_to_trits(ptrit_t const *const ptrits, trit_t *const trits,
                     size_t const index, size_t length) {
  if (length == 0) {
    return;
  }

  trits[0] = (ptrits->low & (1uLL << index))
                 ? ((ptrits->high & (1uLL << index)) ? 0 : -1)
                 : 1;
  ptrits_to_trits(&ptrits[1], &trits[1], index, length - 1);
}
