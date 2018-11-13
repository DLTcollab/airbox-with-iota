/*
 * Copyright (c) 2018 IOTA Stiftung
 * https://github.com/iotaledger/entangled
 *
 * Refer to the LICENSE file for licensing information
 */

#include "hamming.h"
#include <stdio.h>
#include "ptrit.h"
#include "search.h"
#include "trit.h"

short test(PCurl *const curl, unsigned short const security) {
  printf("test 1 a\n");
  unsigned short i, j, k;
  signed short sum;
  printf("test 1 b\n");
  for (i = 0; i < sizeof(ptrit_s) * 8; i++) {
    sum = 0;
  printf("test 1 c\n");

    for (j = 0; j < security; j++) {
      for (k = j * HASH_LENGTH / 3; k < (j + 1) * HASH_LENGTH / 3; k++) {
        if ((curl->state[k].low & (1uLL << i)) == 0) {
          sum--;
        } else if ((curl->state[k].high & (1uLL << i)) == 0) {
          sum++;
        }
      }
  printf("test 1 d\n");

      if (sum == 0 && j < security - 1) {
        sum = 1;
        break;
      }
    }
  printf("test 1 e\n");

    if (sum == 0) {
      return i;
    }
  }
  printf("test 1 f\n");

  return -1;
}

PearlDiverStatus hamming(Curl *const ctx, unsigned short const offset,
                         unsigned short const end,
                         unsigned short const security) {
  return pd_search(ctx, offset, end, test, security);
}
