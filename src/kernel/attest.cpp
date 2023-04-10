#include <stdint.h>

extern "C" {
#include "util.h"
#include <hmac-sha256.h>
}


//TODO: construct `data`: append msg to counter ?
//TODO: use differnt type for counter, as int size is compiler dependent
extern "C" {
	int counter = 0;
	uint8_t myKey[KEY_LEN] = { 0x0 };
	void attest(
	        uint8_t *in_msg_hash,	// Read-Only Vector
	        uint8_t *out_attestation  	// Output Result
	        )
	{
#pragma HLS INTERFACE m_axi port=in_msg_hash bundle=aximm1
#pragma HLS INTERFACE m_axi port=out_attestation bundle=aximm1

	// populate input w/ zeros
	uint8_t in_data[INPUT_LEN] = { 0x0 };
	// stores counter to bytes conversion
	uint8_t counter_bytes[COUNTER_LEN];

	// prepare hash input
	// populate input w/ msg_hash
	for (int i = 0 ; i < INPUT_MSG_HASH_LEN ; i++ ) {
		in_data[i] = in_msg_hash[i];
	}

	// populate input w/ counter (as bytes)
	// transform counter to byte array via shifts
	counter_bytes[0] = (counter >> 24) & 0xFF;
	counter_bytes[1] = (counter >> 16) & 0xFF;
	counter_bytes[2] = (counter >> 8) & 0xFF;
	counter_bytes[3] = counter & 0xFF;

	for (int i = 0 ; i < COUNTER_LEN ; i++ ) {
		in_data[INPUT_MSG_HASH_LEN + i] = counter_bytes[i];
	}

	// increment counter ; NOTE differs from trinc
	counter++;

	// h  = hash({key, val, counter_val});
	// attestation = encrypt(h);
	// return attestation


	//hmac_sha256 (uint8_t out[HMAC_SHA256_DIGEST_SIZE],
	//             const uint8_t *data, size_t data_len,
	//             const uint8_t *key, size_t key_len);
	//
	//             out = out_attestation : binding hash of key to message hash and counter value
	//             data = message hash || counter val
	//             key = key
	//
	//             TODO: what is sent to other party? message, hmac, counter val?

	// TODO: is hmac the correct operation? why does Dimitra use encryption?
	// TODO: asymmetric or symetric crypto? HMAC: symetric -> implement for now
	// due to simplicity
	// NOTE: out attestation MUST have size HMAC_SHA256_DIGEST_SIZE (32)

	hmac_sha256(
		out_attestation,
		in_data,
		INPUT_LEN,
		myKey,
		KEY_LEN
	);

	
	}
}


#ifdef _DEBUG_KERNEL_BUILD
#include <stdio.h>
int
main() {	
	uint8_t in_data[INPUT_MSG_HASH_LEN] = { 0xFF };
	uint8_t out_data[HMAC_SHA256_DIGEST_SIZE] = { 0x00 };
	attest(
		in_data,
		out_data
	);
	
	for (int i = 0; i < HMAC_SHA256_DIGEST_SIZE ; i++) {
		printf("%x", out_data[i]);
	}
	printf("\n");
}
#endif

