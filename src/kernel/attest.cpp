
#include "util.h"

#include <iostream>

// xf
#include <xf_security/hmac.hpp>
#include <xf_security/sha224_256.hpp>
#include <ap_fixed.h>           // ap_uint

// ap_uint: optimized for FPGA hardware
//
#include <cstring>  // string
#include <stdint.h>
#include <stddef.h>

uint32_t counter { 0 };
const ap_uint<8> my_key[key_size] { 0x0 };
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
//        my_key
//    );

    // see https://docs.xilinx.com/r/en-US/Vitis_Libraries/security/guide_L1/hw_api.html_73

    // create objects to instantiate the hmac template stream types
    hls::stream<ap_uint<32>> key_strm;
    hls::stream<ap_uint<32>> msg_strm;
    hls::stream<ap_uint<32>> msg_len_strm;
    hls::stream<bool> e_len_strm;
    hls::stream<ap_uint<32>> hsh_strm;
    hls::stream<bool> e_hsh_strm;

    // set up input data
    key_strm.write(my_key)

//    xf::security::hmac<xf::security::sha256> hmac(meKey, key_size);
//    hmac.update(in_msg_hash, input_msg_hash_size);
//    hmac.update(counter, counter_size);
//    hmac.finalize(out_attestation);
}
