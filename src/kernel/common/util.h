#ifndef UTIL_H
#define UTIL_H

// #include <ap_fixed.h>           // ap_uint
#include <stdint.h>

// KEY RELEVANT CONSTS
const uint32_t key_size { 32 };

// HASH INPUT RELEVANT CONSTS
// https://crypto.stackexchange.com/a/34866
// input size = message hash size (256 bit -> 32 byte) + counter max size (log(int) = 4)
// NOTE: int max size differs per compiler (see https://stackoverflow.com/questions/11438794/is-the-size-of-c-int-2-bytes-or-4-bytes)
const uint32_t input_msg_hash_size { 32 };
const uint32_t counter_size { 4 };

// input size for hmac
const uint32_t input_size { input_msg_hash_size + counter_size } ;

const uint32_t hmac_sha256_digest_size { 32 };

#endif
