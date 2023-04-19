
#include "util.h"

// xf
#include <xf_security/hmac.hpp>
#include <ap_fixed.h>           // ap_uint

// ap_uint: optimized for FPGA hardware

uint32_t counter = 0;
uint8_t myKey[key_size] = { 0x0 };
void SYMMETRIC_ATTEST(
    uint8_t in_msg_hash[input_msg_hash_size],   // Read-Only Vector
    uint8_t out_attestation[hmac_sha256_digest_size]    // Output Result
)
{
#pragma HLS INTERFACE m_axi port=in_msg_hash bundle=aximm1
#pragma HLS INTERFACE m_axi port=out_attestation bundle=aximm1
    
    // NOTE: difference to trinc in counter in paper -> distinguish between counters
//    compute_msg_hmac(
//        out_attestation,
//        in_msg_hash,
//        counter,
//        myKey
//    );

    xf::security::hmac<xf::security::Sha256> hmac(key, KEY_SIZE);
    hmac.update(message, MESSAGE_SIZE);
    hmac.finalize(digest);
}

