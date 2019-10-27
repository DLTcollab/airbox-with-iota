/*
 * Copyright (c) 2018 IOTA Stiftung
 * https://github.com/iotaledger/entangled
 *
 * Refer to the LICENSE file for licensing information
 */

#ifndef __COMMON_CURL_P_SEARCH_H_
#define __COMMON_CURL_P_SEARCH_H_

#include "pearl_diver.h"
#include "curl_p_ptrit.h"
#include "trit.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef intptr_t test_arg_t;
typedef intptr_t test_result_t;
typedef test_result_t (*test_fun_t)(pcurl_t const *, test_arg_t);

PearlDiverStatus pd_search(Curl *ctx, size_t begin, size_t end, test_fun_t test, test_arg_t param);

#ifdef __cplusplus
}
#endif

#endif
