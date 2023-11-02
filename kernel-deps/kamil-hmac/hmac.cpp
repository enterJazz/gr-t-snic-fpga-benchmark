#include "kamil-hmac/hmac.hpp"
#include "kamil-hmac/hmac_org.hpp"
#include <iostream>
// void hmac_internal(uint8_t& in_data[128], uint8_t& out[128])
void hmac_internal() {
   std::cout << __PRETTY_FUNCTION__ << " 1\n";
  uint8_t in_data[32];
  uint8_t out[32];
  hls::stream<AXIS_DATA> input;
  for (auto i = 0; i < 1; i++) {
    AXIS_DATA curWord;
    curWord.tdata = in_data[i];
    if (i == 0)
      curWord.tlast = 1;
    input.write(curWord);
  }
  hls::stream<AXIS_DATA> output;
  hmac(input, output);
   std::cout << __PRETTY_FUNCTION__ << " 2\n";
    AXIS_DATA curWord = output.read();
    out[0] = (uint8_t)curWord.tdata;

   std::cout << __PRETTY_FUNCTION__ << " 3\n";
}
