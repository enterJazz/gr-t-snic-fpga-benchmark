
#define CONTEXT "Example"

#include <stdint.h>
#include <iostream>

extern "C" {
#include <tweetnacl.h>
#include <cstring> // memcpy

    // shamelessly copied from common.cpp
    void prepare_signature_input
    (
        uint8_t out_input[36],
        uint8_t msg_hash[32],
        uint8_t counter_byte_array[3],
        uint32_t counter
    )
    {
        // store counter as byte repr to give as input to signature
        std::memcpy(counter_byte_array, &counter, 4);

        // concat arrays
        std::memcpy(out_input, msg_hash, 32);
        std::memcpy(out_input + 32, counter_byte_array, 4);
    }

int main() {

    uint8_t msg_hash[] {0x3f, 0xc9, 0xb6, 0x89, 0x45, 0x9d, 0x73, 0x8f,
                             0x8c, 0x88, 0xa3, 0xa4, 0x8a, 0xa9, 0xe3, 0x35,
                             0x42, 0x01, 0x6b, 0x7a, 0x40, 0x52, 0xe0, 0x01,
                             0xaa, 0x53, 0x6f, 0xca, 0x74, 0x81, 0x3c, 0xb0};


    unsigned char pk[] {
        0x38, 0xce, 0xa6, 0x7a, 0x78, 0xc5, 0x8e, 0x3d,
        0xa5, 0x5c, 0x79, 0xa7, 0xe6, 0xc3, 0x3d, 0xa4,
        0xe,  0xc0, 0xe9, 0xdf, 0xe7, 0x54, 0x3a, 0x7e,
        0x23, 0xe3, 0xc3, 0xdf, 0xe4, 0xfc, 0xe2, 0x56
    };

    unsigned char sk[] {
        0xa6, 0xb8, 0xdc, 0x1c, 0x85, 0xcf, 0x74, 0x9e,
        0x43, 0xba, 0x6c, 0xf2, 0x7d, 0x3b, 0x28, 0x80,
        0x73, 0x46, 0x34, 0xe7, 0xc4, 0xd7, 0xb1, 0xbc,
        0xd5, 0xe5, 0xa2, 0xc7, 0x9d, 0xb6, 0x80, 0xd2,
        0x38, 0xce, 0xa6, 0x7a, 0x78, 0xc5, 0x8e, 0x3d,
        0xa5, 0x5c, 0x79, 0xa7, 0xe6, 0xc3, 0x3d, 0xa4,
        0xe,  0xc0, 0xe9, 0xdf, 0xe7, 0x54, 0x3a, 0x7e,
        0x23, 0xe3, 0xc3, 0xdf, 0xe4, 0xfc, 0xe2, 0x56
    };



    std::cout << "PUBKEY: "  << std::endl;
    for (int i = 0; i<32; i++) {
        // std::cout << std::hex << std::setfill(' << std::endl;
        printf("0x%x, ", pk[i]);
    }

    std::cout << std::endl;

    std::cout << "PRIVKEY: " << std::endl;

    for (int i = 0; i<64; i++) {
        // std::cout << std::hex << std::setfill(' << std::endl;
        printf("0x%x, ", sk[i]);
    }

    std::cout << std::endl;

    std::cout << "SIGNATURE: " << std::endl;
    uint8_t signature[crypto_sign_BYTES] { 0x0 };
    uint8_t signed_message[36 + crypto_sign_BYTES] { 0x0 };


    // shamelessly copied from ../src/kernel/asym/attest.cpp
    uint8_t counter_byte_array[4] { 0x0 };
    uint8_t sign_input_array[36] { 0x0 };
    uint8_t counter { 0 } ;

    std::cout << "\n\nSIG w/ COUNTER = 0\n" << std::endl;
    prepare_signature_input
    (
        sign_input_array,
        msg_hash,
        counter_byte_array,
        counter
    );

    unsigned long long signed_message_len;

    crypto_sign(signed_message, &signed_message_len, sign_input_array, 36, sk);

    for (int i = 0; i<signed_message_len; i++) {
        // std::cout << std::hex << std::setfill(' << std::endl;
        printf("0x%x, ", signature[i]);
    }

    std::cout << std::endl;

    std::cout << "\n\nSIG w/ COUNTER = 1\n" << std::endl;
    counter = 1;
    prepare_signature_input
    (
        sign_input_array,
        msg_hash,
        counter_byte_array,
        counter
    );

    crypto_sign(signed_message, &signed_message_len, sign_input_array, 36, sk);

    for (int i = 0; i<signed_message_len; i++) {
        printf("0x%x, ", signature[i]);
    }


    int result = crypto_sign_open(
        sign_input_array,
        (long long unsigned *) 36,
        signed_message,
        signed_message_len,
        pk
    );

    std::cout << "\nSIGNED MESSAGE RESULT" << result << std::endl;

}

}
