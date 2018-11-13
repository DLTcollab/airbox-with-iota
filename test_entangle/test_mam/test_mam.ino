/*
 * Copyright (c) 2018 IOTA Stiftung
 * https://github.com/iotaledger/entangled
 *
 * Refer to the LICENSE file for licensing information
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "trit.h"
#include "iss_curl.h"
#include "trit_tryte.h"
#include "mam.h"
#include "merkle.h"

#include "hamming.h"
#include "trit_long.h"
#include "mask.h"

#include <SoftwareSerial.h>


#include "ptrit.h"
#include "ptrit_incr.h"
#include "trit_ptrit.h"
#include "rw_lock.h"
#include "thread.h"
#include "system.h"

void trits_to_ptrits_singlethread(trit_t const *const trits, ptrit_t *const ptrits,
                     size_t const length) {
  int i;
  for (i = 0; i < length; i++) {
    ptrits[i].low = 0x0;
    ptrits[i].high = 0x0;
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

typedef enum { SEARCH_RUNNING, SEARCH_INTERRUPT, SEARCH_FINISHED } SearchStatus;

typedef struct {
  unsigned short index;
  unsigned short offset;
  unsigned short end;
  PCurl curl;
  short (*test)(PCurl *, unsigned short);
  unsigned short param;
  SearchStatus *status;
  rw_lock_handle_t *statusLock;
} SearchInstance;

void init_inst(SearchInstance *const inst, SearchStatus *const status,
               rw_lock_handle_t *const statusLock, unsigned short const index,
               PCurl *const curl, unsigned short const offset,
               unsigned short const end, unsigned short const param,
               short (*test)(PCurl *const, unsigned short const));
void pt_start(thread_handle_t *const tid, SearchInstance *const inst,
              unsigned short const index);
intptr_t run_search_thread(void *const data);
short do_pd_search_singlethread(short (*test)(PCurl *const, unsigned short const),
                   SearchInstance *const inst, PCurl *const copy);

PearlDiverStatus pd_search_singlethread(Curl *const ctx, unsigned short const offset,
                           unsigned short const end,
                           short (*test)(PCurl *const, unsigned short const),
                           unsigned short const param) {
  unsigned short found_thread = 0, n_procs = system_cpu_available();

  intptr_t found_index = -1;
  SearchStatus status = SEARCH_RUNNING;
  rw_lock_handle_t statusLock;

  if (rw_lock_handle_init(&statusLock)) {
    return PEARL_DIVER_ERROR;
  }

  SearchInstance *inst = (SearchInstance *) calloc(n_procs, sizeof(SearchInstance));
  thread_handle_t *tid = (thread_handle_t *) calloc(n_procs, sizeof(thread_handle_t));

  {
    PCurl *curl = new PCurl;
    trits_to_ptrits_singlethread(ctx->state, curl->state, STATE_LENGTH);
    ptrit_offset(&curl->state[offset], 4);
    curl->type = ctx->type;
    init_inst(inst, &status, &statusLock, n_procs, curl, offset + 4, end,
              param, test);
    //free(curl);
  }

  n_procs = 1;
  found_index = run_search_thread_singlethread((void *)&inst[0]);

  /*pt_start(tid, inst, n_procs - 1);

  for (int i = n_procs; --i >= 0;) {
    if (tid[i] == 0) continue;

    if (found_index == -1) {
      thread_handle_join(tid[i], (void *)&found_index);

      if (found_index != -1) {
        found_thread = i;
      }
    } else {
      thread_handle_join(tid[i], NULL);
    }
  }*/

  switch (found_index) {
    case -1:
      free(inst);
      free(tid);

      return PEARL_DIVER_ERROR;
  }

  rw_lock_handle_destroy(&statusLock);

  ptrits_to_trits(&inst[found_thread].curl.state[offset], &ctx->state[offset],
                  found_index, end - offset);

  free(inst);
  free(tid);

  return PEARL_DIVER_SUCCESS;
}

void init_inst(SearchInstance *const inst, SearchStatus *const status,
               rw_lock_handle_t *const statusLock, unsigned short const index,
               PCurl *const curl, unsigned short const offset,
               unsigned short const end, unsigned short const param,
               short (*test)(PCurl *const, unsigned short const)) {
  if (index == 0) {
    return;
  }
  inst->index = index - 1;
  inst->offset = offset;
  inst->end = end;
  inst->param = param;
  inst->status = status;
  inst->statusLock = statusLock;
  inst->test = test;
  
  memcpy(&(inst->curl), curl, sizeof(PCurl));
  init_inst(&inst[1], status, statusLock, index - 1, curl, offset, end, param,
            test);
}

intptr_t run_search_thread_singlethread(void *const data) {
  unsigned short i;
  PCurl *copy = new PCurl;

  SearchInstance *inst = ((SearchInstance *)data);

  for (i = 0; i < inst->index; i++) {
    ptrit_increment(inst->curl.state, inst->offset, HASH_LENGTH);
  }

  intptr_t ret = do_pd_search_singlethread(inst->test, inst, copy);
  free(copy);
  return ret;
}

short do_pd_search_singlethread(short (*test)(PCurl *const, unsigned short const),
                   SearchInstance *const inst, PCurl *const copy) {
  short index;

  SearchStatus status = SEARCH_RUNNING;
  while (status == SEARCH_RUNNING) {
    memcpy(copy, &inst->curl, sizeof(PCurl));
    ptrit_transform(copy);
    index = test_singlethread(copy, inst->param);
    if (index >= 0) {
      rw_lock_handle_wrlock(inst->statusLock);
      *inst->status = SEARCH_FINISHED;
      rw_lock_handle_unlock(inst->statusLock);

      return index;
    }
    ptrit_increment(inst->curl.state, inst->offset + 1, HASH_LENGTH);

    rw_lock_handle_rdlock(inst->statusLock);
    status = *inst->status;
    rw_lock_handle_unlock(inst->statusLock);
  }
  return -1;
  // return do_pd_search(test, inst, copy);
}

short test_singlethread(PCurl *const curl, unsigned short const security) {
  unsigned short i, j, k;
  signed short sum;
  for (i = 0; i < sizeof(ptrit_s) * 8; i++) {
    sum = 0;

    for (j = 0; j < security; j++) {
      for (k = j * HASH_LENGTH / 3; k < (j + 1) * HASH_LENGTH / 3; k++) {
        if ((curl->state[k].low & (1uLL << i)) == 0) {
          sum--;
        } else if ((curl->state[k].high & (1uLL << i)) == 0) {
          sum++;
        }
      }
      if (sum == 0 && j < security - 1) {
        sum = 1;
        break;
      }
    }

    if (sum == 0) {
      return i;
    }
  }

  return -1;
}

PearlDiverStatus hamming_singlethread(Curl *const ctx, unsigned short const offset,
                         unsigned short const end,
                         unsigned short const security) {
  return pd_search_singlethread(ctx, offset, end, test_singlethread, security);
}

static trit_t const merkle_null_hash[HASH_LENGTH] = {0};

int mam_create_singlethread(trit_t *const payload, size_t const payload_length,
               trit_t const *const message, size_t const message_length,
               trit_t const *const side_key, size_t const side_key_length,
               trit_t const *const merkle_tree, size_t const merkle_tree_length,
               size_t const leaf_count, size_t const index,
               trit_t const *const next_root, size_t const start,
               trit_t const *const seed, size_t const security,
               Curl *const enc_curl) {
  if (security > 3) {
    fprintf(stderr, "invalid security %zd\n", security);
    return -1;
  }

  size_t siblings_number = merkle_depth(merkle_tree_length / HASH_LENGTH) - 1;
  size_t enc_index_length = encoded_length(index);
  size_t enc_message_length_length = encoded_length(message_length);
  size_t signature_length = security * ISS_KEY_LENGTH;
  size_t enc_siblings_number_length = encoded_length(siblings_number);
  size_t payload_min_length = enc_index_length + enc_message_length_length +
                              HASH_LENGTH + message_length + (HASH_LENGTH / 3) +
                              signature_length + enc_siblings_number_length +
                              (siblings_number * HASH_LENGTH);
  if (payload_length < payload_min_length) {
    fprintf(stderr, "payload too short: needed %zd, given %zd\n",
            payload_min_length, payload_length);
    return -1;
  }

  size_t offset = 0;
  mam_init_encryption(side_key, side_key_length, merkle_tree, enc_curl);
  // encode index to payload
  encode_long(index, payload + offset, enc_index_length);
  offset += enc_index_length;
  // encode message length to payload
  encode_long(message_length, payload + offset, enc_message_length_length);
  offset += enc_message_length_length;
  curl_absorb(enc_curl, payload, offset);
  // encrypt next root to payload
  mask(payload + offset, next_root, HASH_LENGTH, enc_curl);
  offset += HASH_LENGTH;
  // encrypt message to payload
  mask(payload + offset, message, message_length, enc_curl);
  offset += message_length;
  // encrypt nonce to payload
  Curl curl;
  curl.type = CURL_P_27;
  memcpy(curl.state, enc_curl->state, sizeof(enc_curl->state));
  hamming_singlethread(&curl, 0, HASH_LENGTH / 3, security);
  mask(payload + offset, curl.state, HASH_LENGTH / 3, enc_curl);
  offset += HASH_LENGTH / 3;
  // encrypt signature to payload
  curl_reset(&curl);
  iss_curl_subseed(seed, payload + offset, start + index, &curl);
  iss_curl_key(payload + offset, payload + offset, signature_length, &curl);
  iss_curl_signature(payload + offset, enc_curl->state, 0, payload + offset,
                     signature_length, &curl);
  offset += signature_length;
  // encrypt siblings number to payload
  encode_long(siblings_number, payload + offset, enc_siblings_number_length);
  offset += enc_siblings_number_length;
  // encrypt siblings to payload
  merkle_branch(merkle_tree, payload + offset, merkle_tree_length,
                siblings_number + 1, index, leaf_count);
  offset += siblings_number * HASH_LENGTH;
  size_t to_mask = signature_length + enc_siblings_number_length +
                   siblings_number * HASH_LENGTH;
  mask(payload + offset - to_mask, payload + offset - to_mask, to_mask,
       enc_curl);
  curl_reset(enc_curl);
  return payload_min_length;
}

void test_create(void) {
  char *const seed =
      "TX9XRR9SRCOBMTYDTMKNEIJCSZIMEUPWCNLC9DPDZKKAEMEFVSTEVUFTRUZXEHLULEIYJIEO"
      "WIC9STAHW";
  char *const message =
      "D9999999JDLILILID9999999D9999999GC999999FB999999EA999999D9999999YELILILI"
      "GGOFQGHCMCOC9999WAFEA999UA999999JHTB9999VFLILILIFGOFQGHCCCOC9999ABFEA999"
      "UA999999WGSB9999SGLILILIEGOFQGHCTBOC99999BFEA999UA999999WGSB9999PHLILILI"
      "DGOFQGHCJBOC9999ZAFEA999VA999999WGSB9999N999X999D999H999L999P999T999F999"
      "H999D999";
  char *const side_key =
      "QOLOACG9BNUYLERQTZPPW9VKIOPDRTPMFZCYWGNVKIZJEYBWJDXASOXNDMZGBNYFVBCFBQBX"
      "SCCAFFRIO";

  trit_t seed_trits[3 * strlen(seed) * sizeof(trit_t)];
  trit_t side_key_trits[3 * strlen(side_key) * sizeof(trit_t)];
  trit_t message_trits[3 * strlen(message) * sizeof(trit_t)];

  trytes_to_trits((tryte_t *)seed, seed_trits, strlen(seed));
  trytes_to_trits((tryte_t *)side_key, side_key_trits, strlen(side_key));
  trytes_to_trits((tryte_t *)message, message_trits, strlen(message));

  size_t index = 7;
  size_t message_length = strlen(message) * 3;

  size_t security = 1;
  size_t start = 0;
  size_t count = 16;
  size_t next_start = start + count;
  size_t next_count = 1;
  size_t tree_size = merkle_size((size_t) count);
  size_t next_tree_size = merkle_size((size_t) next_count);
  trit_t merkle_tree[tree_size * HASH_LENGTH];
  trit_t next_root[next_tree_size * HASH_LENGTH];
  int payload_length = payload_min_length(
      message_length, tree_size * HASH_LENGTH, index, security);
  trit_t *payload = (trit_t*) malloc(payload_length * sizeof(trit_t));

  Curl curl;
  curl.type = CURL_P_27;
  init_curl(&curl);
  //TEST_ASSERT_EQUAL_INT(
  //    0, merkle_create(merkle_tree, count, seed_trits, start, security, &curl));
  
  Serial.println("merkle_create");
  Serial.println(0 == merkle_create(merkle_tree, count, seed_trits, start, security, &curl));
  
  Serial.println("curl_reset");
  curl_reset(&curl);
  //TEST_ASSERT_EQUAL_INT(0, merkle_create(next_root, next_count, seed_trits,
  //                                       next_start, security, &curl));

  Serial.println("merkle_create");
  Serial.println(0 == merkle_create(next_root, next_count, seed_trits, next_start, security, &curl));
  
  Serial.println("curl_reset");
  curl_reset(&curl);
  Serial.println("mam_create");
  payload_length = mam_create_singlethread(
      payload, payload_length, message_trits, message_length, side_key_trits,
      strlen(side_key) * 3, merkle_tree, tree_size * HASH_LENGTH, count, index,
      next_root, start, seed_trits, security, &curl);
  //TEST_ASSERT_TRUE(payload_length != -1);
  if (payload_length == -1)
    return;
    
  Serial.println("payload_length");
  Serial.println(payload_length);
    
  Serial.println("curl_reset");
  curl_reset(&curl);
  
  free(payload);
}

void setup() {
  Serial.begin(9600);
  
  delay(2000);
  
  Serial.println("test_create start");
  unsigned long executionTime = micros();
  test_create();
  executionTime = micros() - executionTime;
  Serial.print(executionTime);
  Serial.println(" μs");
  Serial.println("test_create end");
}

void loop() {
  delay(2000);
}
