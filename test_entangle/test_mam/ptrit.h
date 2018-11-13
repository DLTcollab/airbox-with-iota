/*
 * Copyright (c) 2018 IOTA Stiftung
 * https://github.com/iotaledger/entangled
 *
 * Refer to the LICENSE file for licensing information
 */
 
//trinary/prtit.h
#ifndef __COMMON_TRINARY_PTRIT_H_
#define __COMMON_TRINARY_PTRIT_H_

#include "stdint.h"

#define HIGH_BITS 0xFFFFFFFFFFFFFFFF
#define LOW_BITS 0x0000000000000000

typedef int64_t ptrit_s;

typedef struct {
  ptrit_s low;
  ptrit_s high;
} ptrit_t;

#endif

//curl-p/ptrit.h
#ifndef __COMMON_CURL_P_PTRIT_H_
#define __COMMON_CURL_P_PTRIT_H_

#include "const.h"
#include "ptrit_incr.h"

typedef struct {
  ptrit_t state[STATE_LENGTH];
  CurlType type;
} PCurl;

void init_ptrit_curl(PCurl* const ctx);
void ptrit_transform(PCurl* const ctx);
void ptrit_curl_absorb(PCurl* const ctx, ptrit_t const* const trits,
                       size_t const length);
void ptrit_curl_squeeze(PCurl* const ctx, ptrit_t* const trits,
                        size_t const length);
void ptrit_curl_reset(PCurl* const ctx);

#endif
