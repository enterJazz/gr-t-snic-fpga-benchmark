#include "kamil-hmac/hmac.hpp"
#include "kamil-hmac/hmac_org.hpp"
// void hmac_internal(uint8_t& in_data[128], uint8_t& out[128])
void hmac_internal() {

  uint8_t in_data[32];
  uint8_t out[32];
  hls::stream<AXIS_DATA> input;
  for (auto i = 0; i < 32; i++) {
    AXIS_DATA curWord;
    curWord.tdata = in_data[i];
    if (i == 31)
      curWord.tlast = 1;
    input.write(curWord);
  }
  hls::stream<AXIS_DATA> output;
  hmac(input, output);
  for (auto i = 0; i < 32; i++) {
    AXIS_DATA data = output.read();
    out[i] = (uint8_t)data.tdata;
  }
}
