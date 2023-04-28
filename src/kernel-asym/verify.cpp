
extern "C" {
#include "common.h"

#include <monocypher.h>

#include <stdint.h>  // uint8_t, uint32_t
#include <stdbool.h> // bool

    uint32_t counter { 0 };
    const uint8_t signature_legit { 0 };
    void ASYMMETRIC_VERIFY(
            uint8_t in_msg_hash[msg_hash_len],   // Read-Only Vector
            uint8_t in_msg_attestation[attestation_len],   // Read-Only Vector
            uint8_t in_pub_key[public_key_len],
            uint8_t *out_verify_result      // Output Result
            )
    {
#pragma HLS INTERFACE m_axi port=in_msg_hash bundle=aximm1
#pragma HLS INTERFACE m_axi port=in_msg_attestation bundle=aximm1
#pragma HLS INTERFACE m_axi port=in_pub_key bundle=aximm1
#pragma HLS INTERFACE m_axi port=out_verify_result bundle=aximm1

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

        uint8_t result = crypto_eddsa_check
        (
            in_msg_attestation,
            in_pub_key,
            sign_input_array,
            sign_input_len
        );

        if (result == signature_legit)
        {
            counter++;
            out_verify_result[0] = true;
        }
        else
        {
            out_verify_result[0] = false;
        }
    }
}


#ifdef _DEBUG_KERNEL_BUILD
#include <stdio.h>
int
main() {    
    uint8_t in_data[INPUT_MSG_HASH_LEN] = { 0xFF };
    uint8_t in_msg_hmac[HMAC_SHA256_DIGEST_SIZE] = { 0x00 };
    bool out_result;
    verify(
        in_data,
        in_msg_hmac,
        out_result
    );
    
    if (out_result) {
        printf("HMACs match\n");
    } else {
        printf("HMACs DO NOT match\n");
    }

}
#endif

