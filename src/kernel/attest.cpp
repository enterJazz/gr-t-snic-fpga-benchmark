#include <stdint.h> // uint8_t, uint32_t

extern "C" {
#include "util.h"
}


extern "C" {
	uint32_t counter = 0;
	uint8_t myKey[KEY_LEN] = { 0x0 };
	void attest(
	        uint8_t *in_msg_hash,	// Read-Only Vector
	        uint8_t *out_attestation  	// Output Result
	        )
	{
#pragma HLS INTERFACE m_axi port=in_msg_hash bundle=aximm1
#pragma HLS INTERFACE m_axi port=out_attestation bundle=aximm1
	
	compute_msg_hmac(
		out_attestation,
		in_msg_hash,
		counter,
		myKey
	);

	}
}


#ifdef _DEBUG_KERNEL_BUILD
#include <stdio.h> // printf
#include <hmac-sha256.h> // HMAC_SHA256_DIGEST_SIZE
#include <stddef.h> // size_t
int
main() {	
	uint8_t in_data[INPUT_MSG_HASH_LEN] = { 0xFF };
	uint8_t out_data[HMAC_SHA256_DIGEST_SIZE] = { 0x00 };
	attest(
		in_data,
		out_data
	);
	
	for (size_t i = 0; i < HMAC_SHA256_DIGEST_SIZE ; i++) {
		printf("%x", out_data[i]);
	}
	printf("\n");
}
#endif

