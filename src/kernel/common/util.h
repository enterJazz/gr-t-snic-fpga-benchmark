#ifndef UTIL_H
#define UTIL_H

// workaround
#include <iostream>

// xrt includes
#include <ap_fixed.h>   // ap_uint
#include <xf_security/hmac.hpp>
#include <xf_security/sha224_256.hpp>

// removeme
#include <xf_security/md5.hpp>

// sys includes
#include <stdint.h>
#include <stddef.h>     // size_t


const int msg_size   { 4 };
const int hash_size  { 32 };
const int key_l      { 32 };
const int len_w      { 64 };
const int key_w      { ( 8 * msg_size ) };
const int msg_w      { ( 8 * msg_size ) };
const int hash_w     { ( 8 * hash_size ) };
const int block_size { 64 };




// KEY RELEVANT CONSTS
const int key_size { 32 };

// HASH INPUT RELEVANT CONSTS
// https://crypto.stackexchange.com/a/34866
// input size = message hash size (256 bit -> 32 byte) + counter max size (log(int) = 4)
// NOTE: int max size differs per compiler (see https://stackoverflow.com/questions/11438794/is-the-size-of-c-int-2-bytes-or-4-bytes)
const uint8_t input_msg_hash_size { 32 };
const uint8_t counter_size { 4 };

// input size for hmac
const uint8_t input_size { input_msg_hash_size + counter_size } ;

const uint8_t hmac_sha256_digest_size { 32 };


// template specific consts for hmac / sha256
// const int m_width { 32 };
const int m_width { 64 };
const int l_width { 64 };
const int h_width { 256 }; // sha256
// const int h_width { 128 }; // md5

// see ./kernel-deps/L1/include/xf_security_hmac.hpp:304
template <int msgW, int lW, int hshW>
struct sha256_wrapper
{
    static void hash
    (
        hls::stream<ap_uint<msgW>>& msg_strm,
        hls::stream<ap_uint<lW>>& msg_len_strm,
        hls::stream<bool>& e_len_strm,
        hls::stream<ap_uint<hshW>>& hsh_strm,
        hls::stream<bool>& e_hsh_strm
    )
    {
        xf::security::sha256<msg_w>(msg_strm, msg_len_strm, e_len_strm, hsh_strm, e_hsh_strm);
    }
};

// removeme
//
template <int msgW, int lW, int hshW>
struct md5_wrapper {
    static void hash(hls::stream<ap_uint<msgW>>& msgStrm,
                     hls::stream<ap_uint<lW>>& lenStrm,
                     hls::stream<bool>& eLenStrm,
                     hls::stream<ap_uint<hshW>>& hshStrm,
                     hls::stream<bool>& eHshStrm) {
        xf::security::md5(msgStrm, lenStrm, eLenStrm, hshStrm, eHshStrm);
    }
};



#endif
