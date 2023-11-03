#include <ap_axi_sdata.h>
#include <ap_int.h>
#include <hls_stream.h>
#include <stdio.h>

#define WIDTH 512
//(64 * 8)

// typedef ap_axiu <WIDTH, 0, 0, 0> AXIS_DATA;

#define AXI_DATA_BITS 512
#define PID_BITS 6
struct __attribute__((packed)) axisIntf {
  ap_uint<AXI_DATA_BITS> tdata;
  ap_uint<AXI_DATA_BITS / 8> tkeep;
  ap_uint<PID_BITS> tid;
  ap_uint<1> tlast;

  axisIntf() : tdata(0), tkeep(0), tid(0), tlast(0) {}
  axisIntf(ap_uint<AXI_DATA_BITS> tdata, ap_uint<AXI_DATA_BITS / 8> tkeep,
           ap_uint<PID_BITS> tid, ap_uint<1> tlast)
      : tdata(tdata), tkeep(tkeep), tid(tid), tlast(tlast) {}
};
typedef axisIntf AXIS_DATA;

void hmac(hls::stream<AXIS_DATA> &input, hls::stream<AXIS_DATA> &output);
// void sha512_eddsa_verify(hls::stream<AXIS_DATA>& input,
//		hls::stream<ap_uint<1> >& output);
