#include "def_stdint.h"
#include "config.h"
//#include <SoftwareSerial.h>

//mamv2
#include "macros.h"
#include "forced_inline.h"
#include "system.h"
#include "time.h"
#include "thread.h"
#include "rw_lock.h"
#include "memset_safe.h"
#include "errors.h"
#include "error_strings.h"

// third party
#include "uthash.h"
#include "utarray.h"

//keccak
#include "align.h"
#include "brg_endian.h"
#include "SnP-Relaned.h"
#include "KeccakP-1600-SnP.h"
#include "KeccakSpongeWidth1600.h"
#include "KeccakHash.h"
#include "KeccakSponge-common.h"

//common
#include "defs.h"

//trinary
#include "bytes.h"
#include "trit.h"
#include "trits.h"
#include "tryte.h"
#include "ptrit.h"
#include "add.h"
#include "trit_tryte.h"
#include "trit_byte.h"
#include "flex_trit.h"
#include "tryte_ascii.h"
#include "trit_long.h"
#include "tryte_long.h"
#include "ptrit_incr.h"
#include "trit_ptrit.h"

#include "merkle.h"

//model
#include "transaction.h"
#include "inputs.h"
#include "bundle.h"
#include "transfer.h"

//crypto
#include "curl_p_const.h"
#include "curl_p_ptrit.h"
#include "search.h"
#include "general.h"
#include "t27.h"
#include "const.h"
#include "hamming.h"
#include "indices.h"
#include "pearl_diver.h"
#include "troika.h"
#include "ftroika.h"
#include "normalize.h"
#include "v2_iss_curl.h"
#include "kerl.h"
#include "iss_kerl.h"
#include "bigint.h"
#include "converter.h"

//helpers
#include "digest.h"
#include "export.h"
#include "sign.h"

#include "mam_defs.h"
//mam read-only
#include "trit_t_to_mam_msg_read_context_t_map.h"
#include "trit_t_to_mam_msg_write_context_t_map.h"
#include "mam_endpoint_t_set.h"
#include "mam_ntru_pk_t_set.h"
#include "mam_ntru_sk_t_set.h"
#include "mam_channel_t_set.h"
#include "mam_pk_t_set.h"
#include "mam_psk_t_set.h"

//mam trits
#include "buffers.h"
#include "mam_trits.h"

//mam troika
#include "mam_troika.h"

//mam sponge
#include "sponge.h"
#include "spongos_types.h"
#include "spongos.h"

//mam pb3
#include "pb3.h"

//mam prng
#include "prng.h"

//mam wots
#include "wots.h"

//mam mss
#include "mss_common.h"
#include "mss.h"

//mam ntru
#include "ntru_types.h"
#include "ntru.h"
#include "poly_param.h"
#include "poly.h"

//mam psk
#include "psk.h"

//mam prototype
#include "mask.h"
#include "mam.h"

//mam mam
#include "channel.h"
#include "endpoint.h"
#include "message.h"

//mam api
#include "api.h"


bool verify_result(tryte_t *ans, tryte_t *in, int len) {
  int i = 0;
  for (i = 0; i < len; ++i) {
    if(ans[i] != in[i]) {
      return false;
    }
  }
  return true;
}

void test_create() {
  mam_api_t mam;
  tryte_t channel_id[MAM_CHANNEL_ID_TRYTE_SIZE + 1];
  mam_api_init(&mam, (tryte_t *)TRYTES_81_1);

  mam_api_channel_create(&mam, 1, channel_id);
  channel_id[MAM_CHANNEL_ID_TRYTE_SIZE] = '\0';
  bool valid = verify_result(CHID, channel_id, MAM_CHANNEL_ID_TRYTE_SIZE);
  Serial.println(valid);

  mam_api_destroy(&mam);
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
