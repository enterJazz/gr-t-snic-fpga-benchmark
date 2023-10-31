#pragma once

#include <ap_int.h>
#include <hls_stream.h>
#include <stdint.h>

#ifndef __SYNTHESIS__
#include <iostream>
#endif
void helper(uint8_t* h, uint8_t* output);
void hmac(ap_uint<512>& hash, ap_uint<512>& result);
