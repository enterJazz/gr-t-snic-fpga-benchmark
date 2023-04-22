#include "util.h"

// #define _DEBUG_KERNEL_BUILD

#include <iostream>

// xf
#include <xf_security/hmac.hpp>
#include <xf_security/sha224_256.hpp>
#include <xf_security/md5.hpp>
// #include <xf_security_L1.hpp>  // xf::security
#include <ap_fixed.h>           // ap_uint

// ap_uint: optimized for FPGA hardware
//
#include <cstring>  // string
#include <stdint.h>
#include <stddef.h>

#define KEYL 32
#define HASH_SIZE 32


template <int W>
unsigned int string2Strm(std::string data, std::string title, hls::stream<ap_uint<W> >& strm) {
    ap_uint<W> oneWord;
    unsigned int n = 0;
    unsigned int cnt = 0;
    // write msg stream word by word
    for (std::string::size_type i = 0; i < data.length(); i++) {
        if (n == 0) {
            oneWord = 0;
        }
        oneWord.range(7 + 8 * n, 8 * n) = (unsigned)(data[i]);
        n++;
        if (n == W / 8) {
            strm.write(oneWord);
            ++cnt;
            n = 0;
        }
    }
    // deal with the condition that we didn't hit a boundary of the last word
    if (n != 0) {
        strm.write(oneWord);
        ++cnt;
    }
    return cnt;
}


extern "C" {
#include <stdio.h>
uint32_t counter { 0 };
const ap_uint<8> my_key[key_size] { 0x0 };
void SYMMETRIC_ATTEST(
    uint8_t* in_msg_hash,   // Read-Only Vector
    uint8_t* out_attestation      // Output Result
)
{
#pragma HLS INTERFACE m_axi port=in_msg_hash bundle=aximm1
#pragma HLS INTERFACE m_axi port=out_attestation bundle=aximm1
    
    // see https://docs.xilinx.com/r/en-US/Vitis_Libraries/security/guidend_L1/hw_api.html_73

    // create objects to instantiate the hmac template stream types
    hls::stream<ap_uint<key_w>> key_strm;
    hls::stream<ap_uint<64>> len_key_strm;
    hls::stream<ap_uint<msg_w>> msg_strm;
    hls::stream<ap_uint<64>> len_msg_strm;
    hls::stream<bool> end_len_msg_strm;
    hls::stream<ap_uint<hash_w>> hash_strm;
    hls::stream<bool> end_hash_strm;

//    key_strm.write(4);
//    msg_strm.write(5);
//    len_msg_strm.write(1);
//    end_len_msg_strm.write(false);
//    end_len_msg_strm.write(true);

    const char k[] = "key0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000";
    char m[128] { 0 };

    msg_strm.write(m);
    // std::string msg { m };
    len_msg_strm.write(1);

    // xf::security::sha256<m_width>(msg_strm, len_msg_strm, end_len_msg_strm, hash_strm, end_hash_strm);

    int none = string2Strm<key_w>(std::string { k }, "key", key_strm);

    end_len_msg_strm.write(false);
    end_len_msg_strm.write(true);
    end_hash_strm.write(false);
    end_hash_strm.write(true);


    xf::security::hmac<msg_w, len_w, hash_w, key_l, block_size, sha256_wrapper>(key_strm, msg_strm, len_msg_strm, end_len_msg_strm, hash_strm, end_hash_strm);


    // set up input data
//    key_strm.write(my_key);
//
//    // write in msg hash
//    msg_strm.write(in_msg_hash);
//    // write in counter
//    ap_uint<8> counter_bytes[counter_size];
//
//    // populate input w/ counter (as bytes)
//    // transform counter to byte array via shifts
//    counter_bytes[0] = (counter >> 24) & 0xFF;
//    counter_bytes[1] = (counter >> 16) & 0xFF;
//    counter_bytes[2] = (counter >> 8) & 0xFF;
//    counter_bytes[3] = counter & 0xFF;
//
//
//    end_len_msg_strm.write(false);
//    len_msg_strm.write(input_size);
//    end_len_msg_strm.write(true);
//
    // not using verification ; set to false

    // xf::security::hmac<m_width, l_width, h_width, 32, 512, sha256_wrapper>(key_strm, msg_strm, len_msg_strm, end_len_msg_strm, hash_strm, end_hash_strm);
    // xf::security::hmac<32, 32, 64, 128, 64, md5_wrapper>(key_strm, msg_strm, len_msg_strm, end_len_msg_strm, hash_strm, end_hash_strm);
    // xf::security::hmac<m_width, l_width, h_width, 32, 512, md5_wrapper>(key_strm, msg_strm, len_msg_strm, end_len_msg_strm, hash_strm, end_hash_strm);


    // read output data
    // ap_uint<h_width> hash;
    // hash_strm.read(hash);
    // std::cout << "RESULT HASH: " << std::hex << hash << std::endl;
}

}

#ifdef _DEBUG_KERNEL_BUILD
int
main() {    
    uint8_t in_data[input_msg_hash_size] = { 0xFF };
    bool out_result;
    SYMMETRIC_ATTEST(
        in_data,
        out_result
    );
    

}
#endif

