/*
 * Copyright (c) 2018 IOTA Stiftung
 * https://github.com/iotaledger/entangled
 *
 * Refer to the LICENSE file for licensing information
 * 
 * Modifications copyright (c) Zan-Jun Wang
 */

#ifndef __COMMON_CURL_P_HAMMING_H_
#define __COMMON_CURL_P_HAMMING_H_

#include "pearl_diver.h"
#include "trit.h"
#include "ptrit.h"
#include "search.h"

short test(PCurl *const curl, unsigned short const security);

PearlDiverStatus hamming(Curl *const, unsigned short const offset,
                         unsigned short const end,
                         unsigned short const security);

#endif
