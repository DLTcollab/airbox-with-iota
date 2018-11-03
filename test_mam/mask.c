/*
 * Copyright (c) 2018 IOTA Stiftung
 * https://github.com/iotaledger/entangled
 *
 * Refer to the LICENSE file for licensing information
 */

#include <string.h>

#include "add.h"
#include "mask.h"

void mask(trit_t *const dest, trit_t const *const message, size_t const length,
          Curl *const c) {
  size_t chunk_length;
  trit_t chunk[HASH_LENGTH];
  size_t i, j;
  for (i = 0; i < length; i += HASH_LENGTH) {
    chunk_length = length - i < HASH_LENGTH ? length - i : HASH_LENGTH;
    memcpy(chunk, &message[i], chunk_length * sizeof(trit_t));
    for (j = 0; j < chunk_length; j++) {
      dest[i + j] = trit_sum(chunk[j], c->state[j]);
    }
    curl_absorb(c, chunk, chunk_length);
  }
}

void unmask(trit_t *const dest, trit_t const *const cipher, size_t const length,
            Curl *const c) {
  size_t chunk_length;
  size_t i, j;
  for (i = 0; i < length; i += HASH_LENGTH) {
    chunk_length = length - i < HASH_LENGTH ? length - i : HASH_LENGTH;
    for (j = 0; j < chunk_length; j++) {
      dest[i + j] = trit_sum(cipher[i + j], -c->state[j]);
    }
    curl_absorb(c, &dest[i], chunk_length);
  }
}
