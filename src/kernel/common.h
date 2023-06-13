#ifndef COMMON_H
#define COMMON_H

#include <stdint.h>

namespace common::asym
{
    const uint8_t attestation_len { 64 };
    const uint8_t private_key_len { 64 };
    const uint8_t public_key_len { 32 };

    const uint8_t msg_hash_len { 32 };
    const uint8_t counter_len { 4 };
    const uint8_t sign_input_len { msg_hash_len + counter_len };
    const uint8_t signed_message_len { sign_input_len + attestation_len };


    void prepare_signature_input
    (
        uint8_t out_input[sign_input_len],
        uint8_t msg_hash[msg_hash_len],
        uint8_t counter_byte_array[counter_len],
        uint32_t counter
    );

    // corresponding pub and priv key pairs
    // generated via monocypher:
    // crypto_eddsa_key_pair(secret_key, public_key, eddsa_seed)
    const uint8_t pubkey[] {
        0x10, 0xb8, 0x24, 0xdd, 0xef, 0x4a, 0xd3, 0xc0,
        0xe1, 0x2b, 0xd0, 0x54, 0xee, 0xd5, 0x53, 0x9b,
        0x26, 0xc1, 0xce, 0xb6, 0x5c, 0x7a, 0xb7, 0x7b,
        0x27, 0x6f, 0xb5, 0xc7, 0x15, 0x20, 0xb, 0xdd
    };

    const uint8_t privkey[] {
        0x8a, 0x6a, 0x2f, 0xdd, 0x0, 0xbe, 0x41, 0x24,
        0xc6, 0x28, 0x82, 0xfe, 0xb0, 0x1b, 0x6, 0xdf,
        0x8e, 0x3, 0x3f, 0xb3, 0x47, 0x16, 0x1c, 0xa3,
        0x65, 0x6c, 0xb1, 0x75, 0xb4, 0x77, 0x91, 0x88,
        0x10, 0xb8, 0x24, 0xdd, 0xef, 0x4a, 0xd3, 0xc0,
        0xe1, 0x2b, 0xd0, 0x54, 0xee, 0xd5, 0x53, 0x9b,
        0x26, 0xc1, 0xce, 0xb6, 0x5c, 0x7a, 0xb7, 0x7b,
        0x27, 0x6f, 0xb5, 0xc7, 0x15, 0x20, 0xb, 0xdd
    };
}

#endif
