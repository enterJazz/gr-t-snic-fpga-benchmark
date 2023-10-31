extern "C" {
#include "util.h"
#include <hmac-sha256/hmac-sha256.h> // HMAC_SHA256_DIGEST_SIZE, hmac_sha256
#include <kamil-hmac/hmac.hpp>       // HMAC_SHA256_DIGEST_SIZE, hmac_sha256
#include <stdint.h>                  // uint8_t, uint32_t
void compute_msg_hmac(uint8_t out[HMAC_SHA256_DIGEST_SIZE],
                      const uint8_t *msg_hash, uint32_t counter,
                      const uint8_t *key) {
  // populate hmac input w/ zeros
  uint8_t in_data[INPUT_LEN] = {0x0};
  // stores counter to bytes conversion
  uint8_t counter_bytes[COUNTER_LEN];

  // prepare hmac input
  // populate input w/ msg_hash
  for (int i = 0; i < INPUT_MSG_HASH_LEN; i++) {
    in_data[i] = msg_hash[i];
  }

  // populate input w/ counter (as bytes)
  // transform counter to byte array via shifts
  counter_bytes[0] = (counter >> 24) & 0xFF;
  counter_bytes[1] = (counter >> 16) & 0xFF;
  counter_bytes[2] = (counter >> 8) & 0xFF;
  counter_bytes[3] = counter & 0xFF;

  for (int i = 0; i < COUNTER_LEN; i++) {
    in_data[INPUT_MSG_HASH_LEN + i] = counter_bytes[i];
  }

  // increment counter ; NOTE differs from trinc
  counter++;
  /*
     hmac_sha256 (uint8_t out[HMAC_SHA256_DIGEST_SIZE],
     const uint8_t *data, size_t data_len,
     const uint8_t *key, size_t key_len);
     */

  //             out = out_attestation : binding hash of key to message hash and
  //             counter value data = message hash || counter val key = key
  //
  //             TODO: what is sent to other party? message, hmac, counter val?

  // TODO: is hmac the correct operation? why does Dimitra use encryption?
  // TODO: asymmetric or symetric crypto? HMAC: symetric -> implement for now
  // due to simplicity
  // NOTE: out attestation MUST have size HMAC_SHA256_DIGEST_SIZE (32)
  uint8_t out1[128];
  uint8_t in_data1[128];

  hmac_internal();
#if 0
	hls::stream<AXIS_DATA> input;
	for (auto i = 0; i < 32; i++) {
		AXIS_DATA curWord;
		curWord.tdata = in_data[i];
		input.write(curWord);
	}
	hls::stream<AXIS_DATA> output;
	hmac(input, output);
	for (auto i = 0; i < 32; i++) {
		AXIS_DATA data = output.read();
		out[i] = (uint8_t)data.tdata;
	}
#endif
#if 0
	hmac_sha256(
			out,
			in_data,
			INPUT_LEN,
			key,
			KEY_LEN
		   );

#endif
}
}
