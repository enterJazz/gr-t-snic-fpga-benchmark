
//NOTE: if possible, benchmark symmetric vs asymmetric

extern "C"
{
#include "common.h"

#include <monocypher.h>

#include <stdint.h> // uint8_t, uint32_t
#include <cstring> // memcpy

    const uint8_t private_key[private_key_len] { 0xFF };
    uint32_t counter { 0 };
    void ASYMMETRIC_ATTEST
    (
        uint8_t in_msg_hash[msg_hash_len],   // Read-Only Vector
        uint8_t out_attestation[attestation_len]    // Output Result
    )
    {
#pragma HLS INTERFACE m_axi port=in_msg_hash bundle=aximm1
#pragma HLS INTERFACE m_axi port=out_attestation bundle=aximm1
        
        // store counter as byte repr to give as input to signature
        uint8_t counter_byte_array[counter_len] { 0x0 };
        // concat arrays
        uint8_t sign_input_array[sign_input_len] { 0x0 };

        prepare_signature_input
        (
            sign_input_array,
            in_msg_hash,
            counter_byte_array,
            counter
        );

        crypto_eddsa_sign
        (
            out_attestation,
            private_key,
            sign_input_array,
            sign_input_len
        );

        counter++;
    }
}


#ifdef _DEBUG_KERNEL_BUILD
#include <stdio.h> // printf
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

