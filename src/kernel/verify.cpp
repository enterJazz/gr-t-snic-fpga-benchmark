
extern "C" {
#include "common.h"

#include <tweetnacl.h>

#include <stdint.h>  // uint8_t, uint32_t
#include <stdbool.h> // bool
#include <cstring>  // memcpy

    using namespace common::asym;

    uint32_t counter { 0 };
    const int signature_legit { 0 };
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

//        int result = crypto_eddsa_check
//        (
//            in_msg_attestation,
//            in_pub_key,
//            sign_input_array,
//            sign_input_len
//        );
//
        uint8_t signed_message[signed_message_len] { 0x0 };

        std::memcpy(signed_message, sign_input_array, sign_input_len);
        std::memcpy(&signed_message[sign_input_len], in_msg_attestation, attestation_len);


        int result = crypto_sign_open(
            sign_input_array,
            (long long unsigned *) sign_input_len,
            signed_message,
            signed_message_len,
            pubkey
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


