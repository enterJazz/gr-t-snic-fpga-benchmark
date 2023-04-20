
#include "util.h"

#include <iostream>

// xf
#include <xf_security/hmac.hpp>
#include <xf_security/sha224_256.hpp>
// #include <xf_security_L1.hpp>  // xf::security
#include <ap_fixed.h>           // ap_uint

// ap_uint: optimized for FPGA hardware
//
#include <cstring>  // string
#include <stdint.h>
#include <stddef.h>

uint32_t counter { 0 };
const ap_uint<8> my_key[key_size] { 0x0 };
void SYMMETRIC_ATTEST(
    ap_uint<8> in_msg_hash[input_msg_hash_size],   // Read-Only Vector
    ap_uint<8> out_attestation[hmac_sha256_digest_size]    // Output Result
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
    hls::stream<ap_uint<m_width>> key_strm;
    hls::stream<ap_uint<m_width>> msg_strm;
    hls::stream<ap_uint<l_width>> msg_len_strm;
    hls::stream<bool> e_len_strm;
    hls::stream<ap_uint<h_width>> hsh_strm;
    hls::stream<bool> e_hsh_strm;

    // set up input data
    key_strm.write(my_key);

    // write in msg hash
    msg_strm.write(in_msg_hash);
    // write in counter
    ap_uint<8> counter_bytes[counter_size];

    // populate input w/ counter (as bytes)
    // transform counter to byte array via shifts
    counter_bytes[0] = (counter >> 24) & 0xFF;
    counter_bytes[1] = (counter >> 16) & 0xFF;
    counter_bytes[2] = (counter >> 8) & 0xFF;
    counter_bytes[3] = counter & 0xFF;


    msg_len_strm.write(input_size);

    // not using verification ; set to false
    e_len_strm.write(false);
    e_hsh_strm.write(false);

    xf::security::hmac<m_width, l_width, h_width, 32, 512, sha256_wrapper>(key_strm, msg_strm, msg_len_strm, e_len_strm, hsh_strm, e_hsh_strm);
//    hmac.update(in_msg_hash, input_msg_hash_size);
//    hmac.update(counter, counter_size);
//    hmac.finalize(out_attestation);
}
