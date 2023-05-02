#include "common.h"
#include <ap_int.h> 
#include "xf_security/eddsa.hpp"

extern "C"
{

using namespace common::asym;

// xrt includes
// // https://docs.xilinx.com/r/2020.2-English/ug1399-vitis-hls/Arbitrary-Precision-AP-Data-Types

#include <stdint.h> // uint8_t, uint32_t

#include <stdio.h>

    // we only need to load the keys once
    bool keys_loaded { false };
    ap_uint<256> privateKey;
    ap_uint<256> publicKey;
    ap_uint<512> privateKeyHash;
    ap_uint<32> counter { 0 };
    void XF_ASYMMETRIC_ATTEST
    (
        uint8_t in_msg_hash[32],   // Read-Only Vector
        uint8_t out_attestation[64]    // Output Result
    )
    {
#pragma HLS INTERFACE m_axi port=in_msg_hash bundle=aximm1
#pragma HLS INTERFACE m_axi port=out_attestation bundle=aximm1

        xf::security::eddsaEd25519 processor;

        if (!keys_loaded)
        {
            ap_uint<256> tmp = ap_uint<256>(privkey_str);
            for (int i = 0; i < 32; i++)
            {
                int j = 31 - i;
                privateKey.range(i * 8 + 7, i * 8) = tmp.range(j * 8 + 7, j * 8);
            }
            processor.generatePublicKey(privateKey, publicKey, privateKeyHash);
            keys_loaded = true;
        }

        // only debug
        hls::stream<ap_uint<64> > msgStrm;
        hls::stream<ap_uint<128> > lenStrm;
        hls::stream<bool> endLenStrm;
        hls::stream<ap_uint<512> > signatureStrm;
        hls::stream<bool> endSignatureStrm;

        lenStrm.write(1);
        ap_uint<64> msg = ap_uint<64>("0x72");

        ap_uint<256> msg_hash = ap_uint<256>(in_msg_hash);


        msgStrm.write(msg_hash);
        msgStrm.write(counter);
        endLenStrm.write(false);
        endLenStrm.write(true);

        processor.sign(msgStrm, lenStrm, endLenStrm, publicKey, privateKeyHash, signatureStrm, endSignatureStrm);

        endSignatureStrm.read();
        ap_uint<512> sig = signatureStrm.read();
        endSignatureStrm.read();


        // load ap_uint back to out_attestation
        for (int i = 0; i < 64; i++)
        {
            int j = 31 - i;
            out_attestation[i] = sig.range(i * 8 + 7, i * 8);
        }

        counter++;
    }
}
