#ifndef COMMON_H
#define COMMON_H

#include <stdint.h>

const uint8_t msg_hash_len { 32 };
const uint8_t counter_len { 4 };
const uint8_t sign_input_len { msg_hash_len + counter_len };

const uint8_t attestation_len { 64 };
const uint8_t secret_key_len { 64 };

#endif
