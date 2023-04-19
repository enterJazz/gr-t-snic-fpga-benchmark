

extern "C" {
#include <stdint.h> // uint8_t, uint32_t
    void EMPTY(
        uint8_t *in_msg_hash,
        uint8_t *out_attestation    // Output Result
    ) {
#pragma HLS INTERFACE m_axi port=in_msg_hash bundle=aximm1
#pragma HLS INTERFACE m_axi port=out_attestation bundle=aximm1
    }
}


