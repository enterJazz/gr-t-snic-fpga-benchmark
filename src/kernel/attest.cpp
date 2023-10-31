
// NOTE: if possible, benchmark symmetric vs asymmetric

extern "C" {
#include "common.h"
#include "util.h"
#include <cstring>  // memcpy
#include <stdint.h> // uint8_t, uint32_t
#include <tweetnacl.h>

using namespace common::asym;

uint32_t counter{0};
void ASYMMETRIC_ATTEST(uint8_t in_msg_hash[msg_hash_len], // Read-Only Vector
                       uint8_t out_attestation[attestation_len] // Output Result
) {
#pragma HLS INTERFACE m_axi port = in_msg_hash bundle = aximm1
#pragma HLS INTERFACE m_axi port = out_attestation bundle = aximm1

  // store counter as byte repr to give as input to signature
  uint8_t counter_byte_array[counter_len]{0x0};
  // concat arrays
  uint8_t sign_input_array[sign_input_len]{0x0};

  uint8_t signed_message[sign_input_len + attestation_len]{0x0};

  prepare_signature_input(sign_input_array, in_msg_hash, counter_byte_array,
                          counter);

  //        crypto_eddsa_sign
  //        (
  //            out_attestation,
  //            privkey,
  //            sign_input_array,
  //            sign_input_len
  //        );
  //

  //      unsigned long long _signed_message_len;

  //       crypto_sign(signed_message, &_signed_message_len, sign_input_array,
  //       36, privkey);

  auto &myKey = privkey;
  compute_msg_hmac(out_attestation, in_msg_hash, counter, myKey);
  //        std::memcpy(out_attestation, &signed_message[sign_input_len],
  //        attestation_len);

  counter++;
}
}
