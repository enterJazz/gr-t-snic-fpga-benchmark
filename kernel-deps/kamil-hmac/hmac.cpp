#include "kamil-hmac/hmac.hpp"
#include "kamil-hmac/hmac_org.hpp"
#include <cstdint>
#include <iostream>
void hmac_internal(uint8_t data[64], uint8_t out[64]) {

  std::cout << __PRETTY_FUNCTION__ << " 1\n";
  hls::stream<AXIS_DATA> input;
  ap_uint<512> bigWord;

  ap_uint<64> tmp_data;
  tmp_data.range(7, 0) = data[0];
  tmp_data.range(15, 8) = data[1];
  tmp_data.range(23, 16) = data[2];
  tmp_data.range(31, 24) = data[3];
  tmp_data.range(39, 32) = data[4];
  tmp_data.range(47, 40) = data[5];
  tmp_data.range(55, 48) = data[6];
  tmp_data.range(63, 56) = data[7];

  bigWord.range(63, 0) = tmp_data;

  tmp_data.range(7, 0) = data[8];
  tmp_data.range(15, 8) = data[9];
  tmp_data.range(23, 16) = data[10];
  tmp_data.range(31, 24) = data[11];
  tmp_data.range(39, 32) = data[12];
  tmp_data.range(47, 40) = data[13];
  tmp_data.range(55, 48) = data[14];
  tmp_data.range(63, 56) = data[15];

  bigWord.range(127, 64) = tmp_data;

  tmp_data.range(7, 0) = data[16];
  tmp_data.range(15, 8) = data[17];
  tmp_data.range(23, 16) = data[18];
  tmp_data.range(31, 24) = data[19];
  tmp_data.range(39, 32) = data[20];
  tmp_data.range(47, 40) = data[21];
  tmp_data.range(55, 48) = data[22];
  tmp_data.range(63, 56) = data[23];
  bigWord.range(191, 128) = tmp_data;

  tmp_data.range(7, 0) = data[24];
  tmp_data.range(15, 8) = data[25];
  tmp_data.range(23, 16) = data[26];
  tmp_data.range(31, 24) = data[27];
  tmp_data.range(39, 32) = data[28];
  tmp_data.range(47, 40) = data[29];
  tmp_data.range(55, 48) = data[30];
  tmp_data.range(63, 56) = data[31];
  bigWord.range(255, 192) = tmp_data;

  tmp_data.range(7, 0) = data[32];
  tmp_data.range(15, 8) = data[33];
  tmp_data.range(23, 16) = data[34];
  tmp_data.range(31, 24) = data[35];
  tmp_data.range(39, 32) = data[36];
  tmp_data.range(47, 40) = data[37];
  tmp_data.range(55, 48) = data[38];
  tmp_data.range(63, 56) = data[39];
  bigWord.range(319, 256) = tmp_data;

  tmp_data.range(7, 0) = data[40];
  tmp_data.range(15, 8) = data[41];
  tmp_data.range(23, 16) = data[42];
  tmp_data.range(31, 24) = data[43];
  tmp_data.range(39, 32) = data[44];
  tmp_data.range(47, 40) = data[45];
  tmp_data.range(55, 48) = data[46];
  tmp_data.range(63, 56) = data[47];
  bigWord.range(383, 320) = tmp_data;

  tmp_data.range(7, 0) = data[48];
  tmp_data.range(15, 8) = data[49];
  tmp_data.range(23, 16) = data[50];
  tmp_data.range(31, 24) = data[51];
  tmp_data.range(39, 32) = data[52];
  tmp_data.range(47, 40) = data[53];
  tmp_data.range(55, 48) = data[54];
  tmp_data.range(63, 56) = data[55];
  bigWord.range(447, 384) = tmp_data;

  tmp_data.range(7, 0) = data[56];
  tmp_data.range(15, 8) = data[57];
  tmp_data.range(23, 16) = data[58];
  tmp_data.range(31, 24) = data[59];
  tmp_data.range(39, 32) = data[60];
  tmp_data.range(47, 40) = data[61];
  tmp_data.range(55, 48) = data[62];
  tmp_data.range(63, 56) = data[63];
  bigWord.range(511, 448) = tmp_data;

  for (auto i = 0; i < 4; i++) {
    AXIS_DATA curWord;
    curWord.tdata = bigWord;
    curWord.tlast = 0;
    if (i == 3)
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
