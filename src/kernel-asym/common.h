#ifndef COMMON_H
#define COMMON_H

#include <stdint.h>

const uint8_t msg_hash_len { 32 };
const uint8_t counter_len { 4 };
const uint8_t sign_input_len { msg_hash_len + counter_len };

const uint8_t attestation_len { 64 };
const uint8_t private_key_len { 64 };
const uint8_t public_key_len { 32 };

void prepare_signature_input
(
    uint8_t out_input[sign_input_len],
    uint8_t msg_hash[msg_hash_len],
    uint8_t counter_byte_array[counter_len],
    uint32_t counter
);


#endif
