#include <hls_stream.h>

#include <stdint.h>

extern "C" {
    void EMPTY(hls::stream<uint8_t> in,
              hls::stream<uint8_t> out
              ) {
#pragma HLS INTERFACE m_axi port = in bundle = gmem0
#pragma HLS INTERFACE m_axi port = out bundle = gmem0

    }
}
