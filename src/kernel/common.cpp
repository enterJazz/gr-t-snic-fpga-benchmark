
extern "C" {
#include "common.h"

#include <stdint.h> // uint8_t

namespace common::asym {
void prepare_signature_input(uint8_t out_input[sign_input_len],
                             uint8_t msg_hash[msg_hash_len],
                             uint8_t counter_byte_array[counter_len],
                             uint32_t counter) {
  uint8_t counter_bytes[counter_len]{0x0};

  // populate input w/ counter (as bytes)
  // transform counter to byte array via shifts
  counter_bytes[0] = (counter >> 24) & 0xFF;
  counter_bytes[1] = (counter >> 16) & 0xFF;
  counter_bytes[2] = (counter >> 8) & 0xFF;
  counter_bytes[3] = counter & 0xFF;

  // concat arrays
  for (uint8_t i = 0; i < counter_len; i++) {
    out_input[i] = counter_bytes[i];
  }
  for (uint8_t i = counter_len; i < sign_input_len; i++) {
    out_input[i] = msg_hash[i];
  }
}
} // namespace common::asym
}
