
extern "C" {
#include <stdint.h> // uint8_t, uint32_t
#include <stddef.h> // size_t
#include "util.h" // compute_msg_hmac
#include <hmac-sha256.h> // HMAC_SHA256_DIGEST_SIZE
#include <stdbool.h> // bool

    uint32_t counter = 0;
    uint8_t myKey[key_size] = { 0x0 };
    void SYMMETRIC_VERIFY(
            uint8_t *in_msg_hash,   // Read-Only Vector
            uint8_t *in_msg_hmac,   // Read-Only Vector
            bool out_verify_result      // Output Result
            )
    {
#pragma HLS INTERFACE m_axi port=in_msg_hash bundle=aximm1
#pragma HLS INTERFACE m_axi port=in_msg_hmac bundle=aximm1
#pragma HLS INTERFACE m_axi port=out_verify_result bundle=aximm1

    uint8_t own_msg_hmac[HMAC_SHA256_DIGEST_SIZE];

    compute_msg_hmac(
        own_msg_hmac,
        in_msg_hash,
        counter,
        myKey
    );

    // compare hash results
    for (size_t i = 0 ; i < HMAC_SHA256_DIGEST_SIZE ; i++) {
        if (in_msg_hmac[i] != own_msg_hmac[i]) {
            out_verify_result = false;
            return;
        }
    }

    out_verify_result = true;

    }
}


#ifdef _DEBUG_KERNEL_BUILD
#include <stdio.h>
int
main() {    
    uint8_t in_data[input_msg_hash_size] = { 0xFF };
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

