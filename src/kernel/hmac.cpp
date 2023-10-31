extern "C" {
#include "hmac_k.h"
#include <cstring>
#include <stdint.h> // uint8_t, uint32_t
#include <stdio.h>
void RSA(uint8_t in_msg_hash[32],
         uint8_t out_attestation[64] // Output Result
) {
#pragma HLS INTERFACE m_axi port = in_msg_hash bundle = aximm1
#pragma HLS INTERFACE m_axi port = out_attestation bundle = aximm1
  uint8_t out_attestation2[64];
  hmac_k(in_msg_hash, out_attestation2);
  std::memcpy(out_attestation, out_attestation2, 64);
}
}
