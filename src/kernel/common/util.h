

#ifndef UTIL_H
#define UTIL_H

#include <stdint.h> // uint8_t, uint32_t
#include <hmac-sha256.h> // HMAC_SHA256_DIGEST_SIZE

// KEY RELEVANT MACROS
#define KEY_LEN 32

// HASH INPUT RELEVANT MACROS
// https://crypto.stackexchange.com/a/34866
// input len = message hash len (256 bit -> 32 byte) + counter max size len (log(int) = 4)
// NOTE: int max len differs per compiler (see https://stackoverflow.com/questions/11438794/is-the-size-of-c-int-2-bytes-or-4-bytes)
#define INPUT_MSG_HASH_LEN 32
#define COUNTER_LEN 4
#define INPUT_LEN (INPUT_MSG_HASH_LEN + COUNTER_LEN)

void
compute_msg_hmac(
	uint8_t out[HMAC_SHA256_DIGEST_SIZE],
	const uint8_t *msg_hash,
	uint32_t counter,
	const uint8_t *key
);

#endif
