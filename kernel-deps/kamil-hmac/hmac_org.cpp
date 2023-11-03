#include "hmac_org.hpp"

// void sha512_eddsa_verify(hls::stream<AXIS_DATA>& input,
//		hls::stream<ap_uint<1> >& output);

void injectData(hls::stream<AXIS_DATA> &input,
                hls::stream<ap_uint<512>> &data_strm,
                hls::stream<ap_uint<40>> &len_strm, ap_uint<384> key,
                ap_uint<512> &keep, ap_uint<6> &id) {
  /////
  ap_uint<5> tmp_len;
  ap_uint<64> tmp_data;

  ap_uint<512> bigWord = 0;
  ap_uint<40> lenWord = 0;
  lenWord.range(29, 0) = 813965576;
  bigWord.range(383, 0) = key;
  bigWord.range(511, 384) = 0;
  lenWord.range(34, 30) = 16;
  lenWord.range(39, 35) = 0;

  //////

  AXIS_DATA q = input.read();
  ap_uint<1> last = q.tlast;
  ap_uint<512> bbb = q.tdata;

  if (last == 1) {
    len_strm.write(lenWord);
    data_strm.write(bigWord);
  } else {
    do {
      lenWord.range(34, 30) = 8;
      lenWord.range(39, 35) = 8;
      len_strm.write(lenWord);
      lenWord.range(4, 0) = 8;
      lenWord.range(9, 5) = 8;
      lenWord.range(14, 10) = 8;
      lenWord.range(19, 15) = 8;
      lenWord.range(24, 20) = 8;
      bigWord.range(511, 384) = bbb.range(127, 0);
      data_strm.write(bigWord);
      bigWord.range(383, 0) = bbb.range(511, 128);

      q = input.read();
      bbb = q.tdata;
      last = q.tlast;
      if (last == 1) { // last pack
        lenWord.range(29, 25) = 24;
        id = q.tid;
        keep = q.tkeep;
      } else {
        lenWord.range(29, 25) = 8;
      }
    } while (last != 1);
    len_strm.write(lenWord);
    data_strm.write(bigWord);
  }
}

namespace AXI {

namespace xf {
namespace security {
namespace internal {

// @brief Processing block
struct blockType {
  ap_uint<64> M[16];
};

/**
 * @brief Generate 1024-bit processing blocks by padding and appending
 * (pipeline).
 *
 * The algorithm reference is : "Secure Hash Standard", which published by NIST
 * in February 2012. The optimization goal of this function is to yield a
 * 1024-bit block per cycle.
 *
 * @tparam w The bit width of each input message word, default value is 64.
 *
 * @param msg_strm The message being hashed.
 * @param len_strm The message length in byte.
 * @param end_len_strm The flag to signal end of input message stream.
 * @param blk_strm The 1024-bit hash block.
 * @param nblk_strm The number of hash block for this message.
 * @param end_nblk_strm End flag for number of hash block.
 *
 */

static void preProcessing(
    // inputs
    hls::stream<AXIS_DATA> &input,
    // outputs
    hls::stream<blockType> &blk_strm, hls::stream<bool> &end_nblk_strm,
    hls::stream<ap_uint<AXI_DATA_BITS / 8>> &tkeep_strm,
    hls::stream<ap_uint<PID_BITS>> &tid_strm, AXIS_DATA &last_input) {
  bool endFlag;
  ap_uint<128> len = 0;

  ap_uint<512> inputBuffer[2];
  ap_uint<2> bufferIndex = 0;

LOOP_PREPROCESSING_MAIN:
  do {
    // #pragma HLS loop_tripcount min = 1 max = 1 avg = 1
    AXIS_DATA currWord = input.read();
    ap_uint<512> inp512 = currWord.tdata;
    //        std::cout<<"AXI DATA input "<<std::hex<<inp512<<std::endl;
    endFlag = currWord.tlast;
    // message length in byte

    if (!endFlag) {
      len += 64;

      // prepare message length in bit which will be appended at the tail of the
      // block according to the standard
      ap_uint<128> L = 8 * len;

      if (bufferIndex == 2) {
        // we got a third packet
        ////////////////////////////////////////
        ////////////////// FULL BLOCK START

        blockType b0;
#pragma HLS array_partition variable = b0.M complete
        //            std::cout<<"BIG LOOP!\n";
        // this block will hold 16 words (64-bit for each) of message

        ap_uint<512> inp512c = inputBuffer[0];
        //        std::cout<<std::hex<<inp512c<<std::endl;
        ap_uint<5> index;
        for (ap_uint<5> i = 0; i < 16; ++i) {
#pragma HLS unroll
          index = i;
          if (index == 8) {
            index = 0;
            inp512c = inputBuffer[1];
            //                    std::cout<<std::hex<<inp512c<<std::endl;
          } else if (index > 8) {
            index = index - 8;
          }
          ap_uint<64> l = inp512c((7 - index + 1) * 64 - 1, (7 - index) * 64);
          // XXX algorithm assumes big-endian
          b0.M[i] = l;
          //                std::cout<<std::hex<<b0.M[i]<< " ";
        }
        //            std::cout<<std::endl;

        // send the full block
        blk_strm.write(b0);
        end_nblk_strm.write(false);
        ////////////////////////////////////////

        bufferIndex = 0;
      }

      inputBuffer[bufferIndex] = inp512;
      bufferIndex++;
      // BUFFER is FULL and we haven't finished getting data
    } else {
      last_input = currWord;
      tkeep_strm.write(currWord.tkeep);
      tid_strm.write(currWord.tid);
      // prepare message length in bit which will be appended at the tail of the
      // block according to the standard
      ap_uint<128> L = 8 * len;

      // check what to do with buffered data
      if (len == 0) {
        //////////////////////////////////////////////////
        // NO DATA
        // end at block boundary, start with pad 1
        // last block
        blockType b;
#pragma HLS array_partition variable = b.M complete

        // pad 1
        b.M[0] = 0x8000000000000000UL;

      // pad zero words
      LOOP_PAD_13_ZERO_WORDS:
        for (ap_uint<5> i = 1; i < 14; ++i) {
#pragma HLS unroll
          b.M[i] = 0;
        }

        // append L
        b.M[14] = (ap_uint<64>)(0xffffffffffffffffUL & (L >> 64));
        b.M[15] = (ap_uint<64>)(0xffffffffffffffffUL & (L));

        // emit
        blk_strm.write(b);
        end_nblk_strm.write(false);
        //////////////////////////////////////////////////
      } else {
        // gen full block
        if ((len & 0x7f) == 0) {
          ////////////////////////////////////////
          ////////////////// FULL BLOCK START

          blockType b0;
#pragma HLS array_partition variable = b0.M complete
          //            std::cout<<"BIG LOOP!\n";
          // this block will hold 16 words (64-bit for each) of message

          ap_uint<512> inp512c = inputBuffer[0];
          //        std::cout<<std::hex<<inp512c<<std::endl;
          ap_uint<5> index;
          for (ap_uint<5> i = 0; i < 16; ++i) {
#pragma HLS unroll
            index = i;
            if (index == 8) {
              index = 0;
              inp512c = inputBuffer[1];
              //                    std::cout<<std::hex<<inp512c<<std::endl;
            } else if (index > 8) {
              index = index - 8;
            }
            ap_uint<64> l = inp512c((7 - index + 1) * 64 - 1, (7 - index) * 64);
            // XXX algorithm assumes big-endian
            b0.M[i] = l;
            //                std::cout<<std::hex<<b0.M[i]<< " ";
          }
          //            std::cout<<std::endl;

          // send the full block
          blk_strm.write(b0);
          end_nblk_strm.write(false);
          ////////////////////////////////////////
          blockType b;
#pragma HLS array_partition variable = b.M complete

          // pad 1
          b.M[0] = 0x8000000000000000UL;

          // pad zero words
          for (ap_uint<5> i = 1; i < 14; ++i) {
#pragma HLS unroll
            b.M[i] = 0;
          }

          // append L
          b.M[14] = (ap_uint<64>)(0xffffffffffffffffUL & (L >> 64));
          b.M[15] = (ap_uint<64>)(0xffffffffffffffffUL & (L));

          // emit
          blk_strm.write(b);
          end_nblk_strm.write(false);
        } else {
          /////////////////////////////////////////////////////////////
          //////////// NOT FULL BLOCK
          // can pad 1 and append L in current block
          // last block
          blockType b;
#pragma HLS array_partition variable = b.M complete
          ap_uint<5> index = 0;
        LOOP_COPY_TAIL_AND_PAD_1:
          for (ap_uint<5> i = 0; i < 14; ++i) {
#pragma HLS pipeline
            if (i < 8) {
              index = i;
              // pad 1 byte not in this word
              ap_uint<64> l =
                  inputBuffer[0]((7 - index + 1) * 64 - 1, (7 - index) * 64);
              // XXX algorithm assumes big-endian
              b.M[i] = l;
            } else if (i > 8) {
              // pad 1 byte not in this word, and no word to read
              b.M[i] = 0UL;
            } else {
              // pad 1 byte in this word
              b.M[i] = 0x8000000000000000UL;
            }
          }
          // append L
          b.M[14] = (ap_uint<64>)(0xffffffffffffffffUL & (L >> 64));
          b.M[15] = (ap_uint<64>)(0xffffffffffffffffUL & (L));
          //            std::cout<<"BM: ";
          //       	 for (ap_uint<5> i = 0; i < 16; ++i) {
          //       		 std::cout<< b.M[i]<<std::endl;
          //       	 }
          blk_strm.write(b);
          end_nblk_strm.write(false);
          /////////////////////////////////////////////////////////////
        }
      }
      end_nblk_strm.write(true);
    }
  } while (!endFlag);

} // end preProcessing

/**
 *
 * @brief The implementation of right shift operation.
 * The algorithm reference is : "Secure Hash Standard", which published by NIST
 * in February 2012.
 *
 * @tparam w The bit width of input x, default value is 64.
 *
 * @param n Number of bits for input x to be shifted.
 * @param x Word to be shifted.
 *
 */

template <unsigned int w>
ap_uint<w> SHR(
    // inputs
    unsigned int n, ap_uint<w> x) {
#pragma HLS inline

  return (x >> n);

} // end SHR

/**
 *
 * @brief The implementation of rotate right (circular right shift) operation.
 * The algorithm reference is : "Secure Hash Standard", which published by NIST
 * in February 2012.
 *
 * @tparam w The bit width of input x, default value is 64.
 *
 * @param n Number of bits for input x to be shifted.
 * @param x Word to be rotated.
 *
 */

template <unsigned int w>
ap_uint<w> ROTR(
    // inputs
    unsigned int n, ap_uint<w> x) {
#pragma HLS inline

  return ((x >> n) | (x << (w - n)));

} // end ROTR

/**
 *
 * @brief The implementation of Ch(x,y,z).
 * The algorithm reference is : "Secure Hash Standard", which published by NIST
 * in February 2012.
 *
 * @tparam w The bit width of input x, y, and z, default value is 64.
 *
 * @param x The first w-bit input word.
 * @param y The second w-bit input word.
 * @param z The third w-bit input word.
 *
 */

template <unsigned int w>
ap_uint<w> Ch(
    // inputs
    ap_uint<w> x, ap_uint<w> y, ap_uint<w> z) {
#pragma HLS inline

  return ((x & y) ^ ((~x) & z));

} // end Ch

/**
 *
 * @brief The implementation of Maj(x,y,z).
 * The algorithm reference is : "Secure Hash Standard", which published by NIST
 * in February 2012.
 *
 * @tparam w The bit width of input x, y, and z, default value is 64.
 *
 * @param x The first w-bit input word.
 * @param y The second w-bit input word.
 * @param z The third w-bit input word.
 *
 */

template <unsigned int w>
ap_uint<w> Maj(
    // inputs
    ap_uint<w> x, ap_uint<w> y, ap_uint<w> z) {
#pragma HLS inline

  return ((x & y) ^ (x & z) ^ (y & z));

} // end Maj

/**
 *
 * @brief The implementation of upper-case letter sigma 0.
 * The algorithm reference is : "Secure Hash Standard", which published by NIST
 * in February 2012.
 *
 * @tparam w The bit width of input x, default value is 64.
 *
 * @param x The operand.
 *
 */

template <unsigned int w>
ap_uint<w> BSIG0(
    // inputs
    ap_uint<w> x) {
#pragma HLS inline

  return (ROTR<w>(28, x) ^ ROTR<w>(34, x) ^ ROTR<w>(39, x));

} // end BSIG0

/**
 *
 * @brief The implementation of upper-case letter sigma 1.
 * The algorithm reference is : "Secure Hash Standard", which published by NIST
 * in February 2012.
 *
 * @tparam w The bit width of input x, default value is 64.
 *
 * @param x The operand.
 *
 */

template <unsigned int w>
ap_uint<w> BSIG1(
    // inputs
    ap_uint<w> x) {
#pragma HLS inline

  return (ROTR<w>(14, x) ^ ROTR<w>(18, x) ^ ROTR<w>(41, x));

} // end BSIG1

/**
 *
 * @brief The implementation of lower-case letter sigma 0.
 * The algorithm reference is : "Secure Hash Standard", which published by NIST
 * in February 2012.
 *
 * @tparam w The bit width of input x, default value is 64.
 *
 * @param x The operand.
 *
 */

template <unsigned int w>
ap_uint<w> SSIG0(
    // inputs
    ap_uint<w> x) {
#pragma HLS inline

  return (ROTR<w>(1, x) ^ ROTR<w>(8, x) ^ SHR<w>(7, x));

} // end SSIG0

/**
 *
 * @brief The implementation of lower-case letter sigma 1.
 * The algorithm reference is : "Secure Hash Standard", which published by NIST
 * in February 2012.
 *
 * @tparam w The bit width of input x, default value is 64.
 *
 * @param x The operand.
 *
 */

template <unsigned int w>
ap_uint<w> SSIG1(
    // inputs
    ap_uint<w> x) {
#pragma HLS inline

  return (ROTR<w>(19, x) ^ ROTR<w>(61, x) ^ SHR<w>(6, x));

} // end SSIG1

/**
 *
 * @brief Duplicate 1 input stream to 2 output streams
 *
 * @tparam w The bit width of the streams.
 *
 * @param in_strm Input stream to be duplicated.
 * @param in_e_strm End flag of input stream.
 * @param out1_strm The first output stream.
 * @param out1_e_strm End flag of the first output stream.
 * @param out2_strm The second output stream.
 * @param out2_e_strm End flag of the second output stream.
 *
 */

template <unsigned int w>
void dup_strm(
    // stream in
    hls::stream<bool> &in_e_strm,
    // stream out
    hls::stream<bool> &out1_e_strm, hls::stream<bool> &out2_e_strm) {
  bool e = in_e_strm.read();

LOOP_DUP_STREAM:
  while (!e) {
#pragma HLS loop_tripcount min = 1 max = 1 avg = 1
#pragma HLS pipeline II = 1

    out1_e_strm.write(false);
    out2_e_strm.write(false);

    e = in_e_strm.read();
  }

  out1_e_strm.write(true);
  out2_e_strm.write(true);

} // end dup_strm

/**
 *
 * @brief Generate message schedule W (80 words) in stream.
 * The algorithm reference is : "Secure Hash Standard", which published by NIST
 * in February 2012.
 *
 * @tparam w The bit width of message schedule W which defined in the standard,
 * default value is 64.
 *
 * @param blk_strm Message block stream.
 * @param nblk_strm Number of message block stream.
 * @param end_nblk_strm End flag for number of message block stream.
 * @param w_strm The message schedule in stream.
 *
 */
void generateMsgSchedule(
    // inputs
    hls::stream<blockType> &blk_strm, hls::stream<bool> &end_nblk_strm,
    // output
    hls::stream<ap_uint<64>> &w_strm) {
  bool end;

LOOP_GEN_W_NBLK:
  do {
#pragma HLS loop_tripcount min = 1 max = 1 avg = 1
    end = end_nblk_strm.read();
    if (!end) {
      blockType blk = blk_strm.read();
#pragma HLS array_partition variable = blk.M complete

      // message schedule
      ap_uint<64> W[16];
#pragma HLS array_partition variable = W complete

    LOOP_SHA512_GEN_WT16:
      for (ap_uint<5> t = 0; t < 16; t++) {
#pragma HLS pipeline II = 1
        ap_uint<64> Wt = blk.M[t];
        W[t] = Wt;
        w_strm.write(Wt);
      }

    LOOP_SHA1_GEN_WT64:
      for (ap_uint<7> t = 16; t < 80; t++) {
#pragma HLS pipeline II = 1
        ap_uint<64> Wt = SSIG1<64>(W[14]) + W[9] + SSIG0<64>(W[1]) + W[0];
        for (ap_uint<5> i = 0; i < 15; i++) {
          W[i] = W[i + 1];
        }
        W[15] = Wt;
        w_strm.write(Wt);
      }
    }
  } while (!end);

} // end generateMsgSchedule

/**
 *
 * @brief This function performs the computation of SHA-512.
 *
 * The algorithm reference is : "Secure Hash Standard", which published by NIST
 * in February 2012. The implementation is modified for better performance.
 *
 * @tparam w The bit width of each input message word, default value is 64.
 * @tparam hash_width The bit width of hash width, which depends on specific
 * algorithm.
 *
 * @param w_strm Message schedule stream.
 * @param nblk_strm Number of message block stream.
 * @param end_nblk_strm End flag for number of message block stream.
 * @param digest_strm Output digest stream.
 * @param digest_strm End flag for output digest stream.
 *
 */

template <unsigned int w, unsigned int hash_width>
void SHA512Digest(
    // inputs
    hls::stream<ap_uint<64>> &w_strm, hls::stream<bool> &end_nblk_strm,
    hls::stream<ap_uint<AXI_DATA_BITS / 8>> &tkeep_strm,
    hls::stream<ap_uint<PID_BITS>> &tid_strm,
    // outputs
    hls::stream<AXIS_DATA> &output) {
  // the eighty constant 64-bit words of SHA-384, SHA-512, and SHA-512/t
  static const ap_uint<64> K[80] = {
      0x428a2f98d728ae22UL, 0x7137449123ef65cdUL, 0xb5c0fbcfec4d3b2fUL,
      0xe9b5dba58189dbbcUL, 0x3956c25bf348b538UL, 0x59f111f1b605d019UL,
      0x923f82a4af194f9bUL, 0xab1c5ed5da6d8118UL, 0xd807aa98a3030242UL,
      0x12835b0145706fbeUL, 0x243185be4ee4b28cUL, 0x550c7dc3d5ffb4e2UL,
      0x72be5d74f27b896fUL, 0x80deb1fe3b1696b1UL, 0x9bdc06a725c71235UL,
      0xc19bf174cf692694UL, 0xe49b69c19ef14ad2UL, 0xefbe4786384f25e3UL,
      0x0fc19dc68b8cd5b5UL, 0x240ca1cc77ac9c65UL, 0x2de92c6f592b0275UL,
      0x4a7484aa6ea6e483UL, 0x5cb0a9dcbd41fbd4UL, 0x76f988da831153b5UL,
      0x983e5152ee66dfabUL, 0xa831c66d2db43210UL, 0xb00327c898fb213fUL,
      0xbf597fc7beef0ee4UL, 0xc6e00bf33da88fc2UL, 0xd5a79147930aa725UL,
      0x06ca6351e003826fUL, 0x142929670a0e6e70UL, 0x27b70a8546d22ffcUL,
      0x2e1b21385c26c926UL, 0x4d2c6dfc5ac42aedUL, 0x53380d139d95b3dfUL,
      0x650a73548baf63deUL, 0x766a0abb3c77b2a8UL, 0x81c2c92e47edaee6UL,
      0x92722c851482353bUL, 0xa2bfe8a14cf10364UL, 0xa81a664bbc423001UL,
      0xc24b8b70d0f89791UL, 0xc76c51a30654be30UL, 0xd192e819d6ef5218UL,
      0xd69906245565a910UL, 0xf40e35855771202aUL, 0x106aa07032bbd1b8UL,
      0x19a4c116b8d2d0c8UL, 0x1e376c085141ab53UL, 0x2748774cdf8eeb99UL,
      0x34b0bcb5e19b48a8UL, 0x391c0cb3c5c95a63UL, 0x4ed8aa4ae3418acbUL,
      0x5b9cca4f7763e373UL, 0x682e6ff3d6b2b8a3UL, 0x748f82ee5defb2fcUL,
      0x78a5636f43172f60UL, 0x84c87814a1f0ab72UL, 0x8cc702081a6439ecUL,
      0x90befffa23631e28UL, 0xa4506cebde82bde9UL, 0xbef9a3f7b2c67915UL,
      0xc67178f2e372532bUL, 0xca273eceea26619cUL, 0xd186b8c721c0c207UL,
      0xeada7dd6cde0eb1eUL, 0xf57d4f7fee6ed178UL, 0x06f067aa72176fbaUL,
      0x0a637dc5a2c898a6UL, 0x113f9804bef90daeUL, 0x1b710b35131c471bUL,
      0x28db77f523047d84UL, 0x32caab7b40c72493UL, 0x3c9ebe0a15c9bebcUL,
      0x431d67c49c100d4cUL, 0x4cc5d4becb3e42b6UL, 0x597f299cfc657e2aUL,
      0x5fcb6fab3ad6faecUL, 0x6c44198c4a475817UL,
  };
#pragma HLS array_partition variable = K complete
#pragma HLS bind_storage variable = K type = ROM_1P impl = LUTRAM

  bool end;

  // the initial hash values, default value is SHA-512
  ap_uint<64> H[8];
  if (hash_width == 384) {
    // SHA-384
    H[0] = 0xcbbb9d5dc1059ed8UL;
    H[1] = 0x629a292a367cd507UL;
    H[2] = 0x9159015a3070dd17UL;
    H[3] = 0x152fecd8f70e5939UL;
    H[4] = 0x67332667ffc00b31UL;
    H[5] = 0x8eb44a8768581511UL;
    H[6] = 0xdb0c2e0d64f98fa7UL;
    H[7] = 0x47b5481dbefa4fa4UL;
  } else if (hash_width == 256) {
    // SHA-512/256
    H[0] = 0x22312194FC2BF72CUL;
    H[1] = 0x9F555FA3C84C64C2UL;
    H[2] = 0x2393B86B6F53B151UL;
    H[3] = 0x963877195940EABDUL;
    H[4] = 0x96283EE2A88EFFE3UL;
    H[5] = 0xBE5E1E2553863992UL;
    H[6] = 0x2B0199FC2C85B8AAUL;
    H[7] = 0x0EB72DDC81C52CA2UL;
  } else if (hash_width == 224) {
    // SHA-512/224
    H[0] = 0x8C3D37C819544DA2UL;
    H[1] = 0x73E1996689DCD4D6UL;
    H[2] = 0x1DFAB7AE32FF9C82UL;
    H[3] = 0x679DD514582F9FCFUL;
    H[4] = 0x0F6D2B697BD44DA8UL;
    H[5] = 0x77E36F7304C48942UL;
    H[6] = 0x3F9D85A86A1D36C8UL;
    H[7] = 0x1112E6AD91D692A1UL;
  } else {
    // SHA-512
    H[0] = 0x6a09e667f3bcc908UL;
    H[1] = 0xbb67ae8584caa73bUL;
    H[2] = 0x3c6ef372fe94f82bUL;
    H[3] = 0xa54ff53a5f1d36f1UL;
    H[4] = 0x510e527fade682d1UL;
    H[5] = 0x9b05688c2b3e6c1fUL;
    H[6] = 0x1f83d9abfb41bd6bUL;
    H[7] = 0x5be0cd19137e2179UL;
  }
#pragma HLS array_partition variable = H complete

LOOP_SHA1_DIGEST_NBLK:
  do {
#pragma HLS loop_tripcount min = 1 max = 1 avg = 1

    end = end_nblk_strm.read();
    if (!end) {

      // load working variables
      ap_uint<w> a = H[0];
      ap_uint<w> b = H[1];
      ap_uint<w> c = H[2];
      ap_uint<w> d = H[3];
      ap_uint<w> e = H[4];
      ap_uint<w> f = H[5];
      ap_uint<w> g = H[6];
      ap_uint<w> h = H[7];

    // update working variables accordingly
    LOOP_SHA1_UPDATE_80_ROUNDS:
      for (ap_uint<7> t = 0; t < 80; t++) {
#pragma HLS pipeline II = 2
        ap_uint<w> Wt = w_strm.read();
#if !defined(__SYNTHESIS__) && __XF_SECURITY_SHA512_T_DEBUG__ == 1
        std::cout << "W[" << std::dec << t << "] = " << std::hex << Wt
                  << std::endl;
#endif
        ap_uint<w> T1 = h + BSIG1<w>(e) + Ch<w>(e, f, g) + K[t] + Wt;
        ap_uint<w> T2 = BSIG0<w>(a) + Maj<w>(a, b, c);
        h = g;
        g = f;
        f = e;
        e = d + T1;
        d = c;
        c = b;
        b = a;
        a = T1 + T2;
#if !defined(__SYNTHESIS__) && __XF_SECURITY_SHA512_T_DEBUG__ == 1
        std::cout << "a = " << std::hex << a << std::endl;
        std::cout << "b = " << std::hex << b << std::endl;
        std::cout << "c = " << std::hex << c << std::endl;
        std::cout << "d = " << std::hex << d << std::endl;
        std::cout << "e = " << std::hex << e << std::endl;
        std::cout << "f = " << std::hex << f << std::endl;
        std::cout << "g = " << std::hex << g << std::endl;
        std::cout << "h = " << std::hex << h << std::endl;
#endif
      }

      // increment internal states with updated working variables
      H[0] = a + H[0];
      H[1] = b + H[1];
      H[2] = c + H[2];
      H[3] = d + H[3];
      H[4] = e + H[4];
      H[5] = f + H[5];
      H[6] = g + H[6];
      H[7] = h + H[7];
#if !defined(__SYNTHESIS__) && __XF_SECURITY_SHA512_T_DEBUG__ == 1
      std::cout << "H[0] = " << std::hex << H[0] << std::endl;
      std::cout << "H[1] = " << std::hex << H[1] << std::endl;
      std::cout << "H[2] = " << std::hex << H[2] << std::endl;
      std::cout << "H[3] = " << std::hex << H[3] << std::endl;
      std::cout << "H[4] = " << std::hex << H[4] << std::endl;
      std::cout << "H[5] = " << std::hex << H[5] << std::endl;
      std::cout << "H[6] = " << std::hex << H[6] << std::endl;
      std::cout << "H[7] = " << std::hex << H[7] << std::endl;
#endif
    }
  } while (!end);

  // emit digest
  ap_uint<8 * 64> digest;
LOOP_SHA512_EMIT:
  for (ap_uint<4> i = 0; i < 8; i++) {
#pragma HLS unroll
    ap_uint<64> l = H[7 - i];
    // XXX shift algorithm's big-endian to HLS's little-endian
    ap_uint<8> byte0 = ((l >> 56) & 0xff);
    ap_uint<8> byte1 = ((l >> 48) & 0xff);
    ap_uint<8> byte2 = ((l >> 40) & 0xff);
    ap_uint<8> byte3 = ((l >> 32) & 0xff);
    ap_uint<8> byte4 = ((l >> 24) & 0xff);
    ap_uint<8> byte5 = ((l >> 16) & 0xff);
    ap_uint<8> byte6 = ((l >> 8) & 0xff);
    ap_uint<8> byte7 = (l & 0xff);
    digest.range(64 * i + 64 - 1, 64 * i) =
        ((ap_uint<64>)byte7) | (((ap_uint<64>)byte6) << 8) |
        (((ap_uint<64>)byte5) << 16) | (((ap_uint<64>)byte4) << 24) |
        (((ap_uint<64>)byte3) << 32) | (((ap_uint<64>)byte2) << 40) |
        (((ap_uint<64>)byte1) << 48) | (((ap_uint<64>)byte0) << 56);
  }
  // obtain the digest by trancating the left-most hash_width bits of 512-bit
  // hash value
  AXIS_DATA outWord;
  outWord.tdata = digest.range(hash_width - 1, 0);

  ap_uint<AXI_DATA_BITS / 8> tkeep = tkeep_strm.read();
  ap_uint<PID_BITS> tid = tid_strm.read();
  outWord.tkeep = tkeep;
  outWord.tid = tid;
  outWord.tlast = 1;
  output.write(outWord);

} // end SHA512Digest

/**
 *
 * @brief Top function of SHA-512.
 *
 * The algorithm reference is : "Secure Hash Standard", which published by NIST
 * in February 2012. The implementation dataflows the pre-processing part and
 * message digest part.
 *
 * @tparam w The bit width of each input message word, default value is 64.
 * @tparam hash_width The bit width of hash width, which depends on specific
 * algorithm.
 *
 * @param msg_strm The message being hashed.
 * @param len_strm The message length in byte.
 * @param end_len_strm The flag to signal end of input message stream.
 * @param digest_strm Output digest stream.
 * @param end_digest_strm End flag for output digest stream.
 *
 */

template <unsigned int w, unsigned int hash_width>
void sha512Top(
    // inputs
    hls::stream<AXIS_DATA> &input,
    // outputs
    hls::stream<AXIS_DATA> &output, AXIS_DATA &last_input) {
#pragma HLS dataflow

  // 1024-bit processing block stream
  hls::stream<blockType> blk_strm("blk_strm");
#pragma HLS stream variable = blk_strm depth = 32
#pragma HLS resource variable = blk_strm core = FIFO_LUTRAM

  // number of blocks stream
  hls::stream<ap_uint<128>> nblk_strm("nblk_strm");
#pragma HLS stream variable = nblk_strm depth = 32
#pragma HLS resource variable = nblk_strm core = FIFO_LUTRAM
  hls::stream<ap_uint<128>> nblk_strm1("nblk_strm1");
#pragma HLS stream variable = nblk_strm1 depth = 32
#pragma HLS resource variable = nblk_strm1 core = FIFO_LUTRAM
  hls::stream<ap_uint<128>> nblk_strm2("nblk_strm2");
#pragma HLS stream variable = nblk_strm2 depth = 32
#pragma HLS resource variable = nblk_strm2 core = FIFO_LUTRAM

  // end flag of number of blocks stream
  hls::stream<bool> end_nblk_strm("end_nblk_strm");
#pragma HLS stream variable = end_nblk_strm depth = 32
#pragma HLS resource variable = end_nblk_strm core = FIFO_LUTRAM
  hls::stream<bool> end_nblk_strm1("end_nblk_strm1");
#pragma HLS stream variable = end_nblk_strm1 depth = 32
#pragma HLS resource variable = end_nblk_strm1 core = FIFO_LUTRAM
  hls::stream<bool> end_nblk_strm2("end_nblk_strm2");
#pragma HLS stream variable = end_nblk_strm2 depth = 32
#pragma HLS resource variable = end_nblk_strm2 core = FIFO_LUTRAM

  // message schedule stream
  hls::stream<ap_uint<64>> w_strm("w_strm");
#pragma HLS stream variable = w_strm depth = 32
#pragma HLS resource variable = w_strm core = FIFO_LUTRAM

  hls::stream<ap_uint<AXI_DATA_BITS / 8>> tkeep_strm;
#pragma HLS stream variable = tkeep_strm depth = 32
#pragma HLS resource variable = tkeep_strm core = FIFO_LUTRAM
  hls::stream<ap_uint<PID_BITS>> tid_strm;
#pragma HLS stream variable = tid_strm depth = 32
#pragma HLS resource variable = tid_strm core = FIFO_LUTRAM

  // padding and appending message words into blocks
  preProcessing(input, blk_strm, end_nblk_strm, tkeep_strm, tid_strm,
                last_input);

  // duplicate number of block stream and its end flag stream
  dup_strm<128>(end_nblk_strm, end_nblk_strm1, end_nblk_strm2);

  // generate the message schedule in stream
  generateMsgSchedule(blk_strm, end_nblk_strm1, w_strm);
  //
  //    // digest precessing blocks into hash value
  SHA512Digest<64, hash_width>(w_strm, end_nblk_strm2, tkeep_strm, tid_strm,
                               output);

} // end sha512Top

void switchEndian(ap_uint<64> &l) {
  l = ((0x00000000000000ffUL & l) << 56) | ((0x000000000000ff00UL & l) << 40) |
      ((0x0000000000ff0000UL & l) << 24) | ((0x00000000ff000000UL & l) << 8) |
      ((0x000000ff00000000UL & l) >> 8) | ((0x0000ff0000000000UL & l) >> 24) |
      ((0x00ff000000000000UL & l) >> 40) | ((0xff00000000000000UL & l) >> 56);
}

static void preProcessingOneTripForHMAC(
    ap_uint<2> op_type, ap_uint<64> opad_buffer[16], ap_uint<384> digest_input,
    hls::stream<ap_uint<512>> &msg_strm, hls::stream<ap_uint<40>> &len_strm,
    hls::stream<ap_uint<64 + 1>> &blk_strm) {
  ap_uint<128> total_len = 192;
  ap_uint<4> counter = 0;
  ap_uint<5> tmp_len;
  ap_uint<40> HUH;
  ap_uint<512> bigHUH;
  if (op_type == 1) { // for "ipad + msg" hash

    HUH = len_strm.read();

    bigHUH = msg_strm.read();

    total_len = 128; // last 8 will be add during pad "1"

    ap_uint<64> l;
    ap_uint<64> ll0, ll1, ll2, ll3, ll4, ll5, ll6, ll7;
    ap_uint<64> ipad;
    ap_uint<64> opad;

    ll0 = bigHUH.range(63, 0);
    ll1 = bigHUH.range(127, 64);
    ll2 = bigHUH.range(191, 128);
    ll3 = bigHUH.range(255, 192);
    ll4 = bigHUH.range(319, 256);
    ll5 = bigHUH.range(383, 320);
    ipad = ll0 ^ ap_uint<64>("0x3636363636363636");
    opad = ll0 ^ ap_uint<64>("0x5c5c5c5c5c5c5c5c");
#ifdef SENDIAN
    switchEndian(ipad);
#endif
    blk_strm.write(ipad);
    opad_buffer[0] = opad;
    ipad = ll1 ^ ap_uint<64>("0x3636363636363636");
    opad = ll1 ^ ap_uint<64>("0x5c5c5c5c5c5c5c5c");
#ifdef SENDIAN
    switchEndian(ipad);
#endif
    blk_strm.write(ipad);
    opad_buffer[1] = opad;
    ipad = ll2 ^ ap_uint<64>("0x3636363636363636");
    opad = ll2 ^ ap_uint<64>("0x5c5c5c5c5c5c5c5c");
#ifdef SENDIAN
    switchEndian(ipad);
#endif
    blk_strm.write(ipad);
    opad_buffer[2] = opad;
    ipad = ll3 ^ ap_uint<64>("0x3636363636363636");
    opad = ll3 ^ ap_uint<64>("0x5c5c5c5c5c5c5c5c");
#ifdef SENDIAN
    switchEndian(ipad);
#endif
    blk_strm.write(ipad);
    opad_buffer[3] = opad;
    ipad = ll4 ^ ap_uint<64>("0x3636363636363636");
    opad = ll4 ^ ap_uint<64>("0x5c5c5c5c5c5c5c5c");
#ifdef SENDIAN
    switchEndian(ipad);
#endif
    blk_strm.write(ipad);
    opad_buffer[4] = opad;
    ipad = ll5 ^ ap_uint<64>("0x3636363636363636");
    opad = ll5 ^ ap_uint<64>("0x5c5c5c5c5c5c5c5c");
#ifdef SENDIAN
    switchEndian(ipad);
#endif
    blk_strm.write(ipad);
    opad_buffer[5] = opad;

    tmp_len = HUH.range(34, 30);
    total_len += tmp_len;
    if (HUH.range(34, 30) == 16 && HUH.range(29, 25) == 24) { // TODO
      l = 0;
      ipad = ap_uint<64>("0x3636363636363636");
      opad = ap_uint<64>("0x5c5c5c5c5c5c5c5c");
#ifdef SENDIAN
      switchEndian(ipad);
#endif
      blk_strm.write(ipad);
      opad_buffer[6] = opad;
      blk_strm.write(ipad);
      opad_buffer[7] = opad;
      blk_strm.write(ipad);
      opad_buffer[8] = opad;
      blk_strm.write(ipad);
      opad_buffer[9] = opad;
      blk_strm.write(ipad);
      opad_buffer[10] = opad;
      blk_strm.write(ipad);
      opad_buffer[11] = opad;
      blk_strm.write(ipad);
      opad_buffer[12] = opad;
      blk_strm.write(ipad);
      opad_buffer[13] = opad;
      blk_strm.write(ipad);
      opad_buffer[14] = opad;
      blk_strm.write(ipad);
      opad_buffer[15] = opad;

      counter++;
      blk_strm.write("9223372036854775808");
    } else {
      total_len += 48;
      counter += 7;
      ll6 = bigHUH.range(447, 384);
      ll7 = bigHUH.range(511, 448);
#ifdef SENDIAN
      switchEndian(ll6);
      switchEndian(ll7);
#endif
      ipad = ap_uint<64>("0x3636363636363636");
#ifdef SENDIAN
      switchEndian(ipad);
#endif
      opad_buffer[6] = ap_uint<64>("0x5c5c5c5c5c5c5c5c");
      blk_strm.write(ipad);
      opad_buffer[7] = ap_uint<64>("0x5c5c5c5c5c5c5c5c");
      blk_strm.write(ipad);
      opad_buffer[8] = ap_uint<64>("0x5c5c5c5c5c5c5c5c");
      blk_strm.write(ipad);
      opad_buffer[9] = ap_uint<64>("0x5c5c5c5c5c5c5c5c");
      blk_strm.write(ipad);
      opad_buffer[10] = ap_uint<64>("0x5c5c5c5c5c5c5c5c");
      blk_strm.write(ipad);
      opad_buffer[11] = ap_uint<64>("0x5c5c5c5c5c5c5c5c");
      blk_strm.write(ipad);
      opad_buffer[12] = ap_uint<64>("0x5c5c5c5c5c5c5c5c");
      blk_strm.write(ipad);
      opad_buffer[13] = ap_uint<64>("0x5c5c5c5c5c5c5c5c");
      blk_strm.write(ipad);
      opad_buffer[14] = ap_uint<64>("0x5c5c5c5c5c5c5c5c");
      blk_strm.write(ipad);
      opad_buffer[15] = ap_uint<64>("0x5c5c5c5c5c5c5c5c");
      blk_strm.write(ipad);
      blk_strm.write(ll6);
      blk_strm.write(ll7);

      bigHUH = msg_strm.read();
      ll0 = bigHUH.range(63, 0);
      ll1 = bigHUH.range(127, 64);
      ll2 = bigHUH.range(191, 128);
      ll3 = bigHUH.range(255, 192);
      ll4 = bigHUH.range(319, 256);
#ifdef SENDIAN
      switchEndian(ll0);
      switchEndian(ll1);
      switchEndian(ll2);
      switchEndian(ll3);
      switchEndian(ll4);
#endif
      blk_strm.write(ll0);
      blk_strm.write(ll1);
      blk_strm.write(ll2);
      blk_strm.write(ll3);
      blk_strm.write(ll4);

      HUH = len_strm.read();
      tmp_len = HUH.range(29, 25);

      while (tmp_len[4] != 1) {

//    #pragma HLS pipeline II = 1
#pragma HLS unroll // kamil

        //#pragma HLS PERFORMANCE target_ti=1000 //KAMIL
        total_len += 56;
        counter += 8;

        ll5 = bigHUH.range(383, 320);
        ll6 = bigHUH.range(447, 384);
        ll7 = bigHUH.range(511, 448);
#ifdef SENDIAN
        switchEndian(ll5);
        switchEndian(ll6);
        switchEndian(ll7);
#endif
        blk_strm.write(ll5);
        blk_strm.write(ll6);
        blk_strm.write(ll7);

        bigHUH = msg_strm.read();
        ll0 = bigHUH.range(63, 0);
        ll1 = bigHUH.range(127, 64);
        ll2 = bigHUH.range(191, 128);
        ll3 = bigHUH.range(255, 192);
        ll4 = bigHUH.range(319, 256);
#ifdef SENDIAN
        switchEndian(ll0);
        switchEndian(ll1);
        switchEndian(ll2);
        switchEndian(ll3);
        switchEndian(ll4);
#endif
        blk_strm.write(ll0);
        blk_strm.write(ll1);
        blk_strm.write(ll2);
        blk_strm.write(ll3);
        blk_strm.write(ll4);

        total_len += tmp_len;

        HUH = len_strm.read();
        tmp_len = HUH.range(29, 25);
      }
      // pad "1"
      {
        ll5 = bigHUH.range(383, 320);
        l = ll5;
#ifdef SENDIAN
        switchEndian(l);
#endif
        ap_uint<4> last_len = tmp_len.range(3, 0);
        total_len += last_len;
        if (last_len == 8) {
          blk_strm.write(l);
          l = 0;
          counter++;
        }
        l.range(63 - 8 * last_len.range(2, 0), 56 - 8 * last_len.range(2, 0)) =
            ap_uint<8>("0x80");
        blk_strm.write(l);
        counter++;
      }
    }

    // pad "0"
    {
      ap_uint<5> z_rounds;
      if (counter < 15) {
        z_rounds = 14 - counter;
      } else {
        z_rounds = 30 - counter;
      }
      for (ap_uint<5> z = 0; z < z_rounds; z++) {
        blk_strm.write(0);
      }
    }
    // pad "len"
    {
      total_len <<= 3;
      ap_uint<65> L = total_len.range(127, 64);
      blk_strm.write(L);
      L = total_len.range(63, 0);
      L[64] = 1;
      if (HUH.range(34, 30) == 16 && HUH.range(29, 25) == 24)
        L = "18446744073709552640";
      blk_strm.write(L);
    }
  } else if (op_type == 2) { // for "opad + hash" hash

    ap_uint<64> l;
    l = opad_buffer[0];
#ifdef SENDIAN
    switchEndian(l);
#endif
    blk_strm.write(l);
    l = opad_buffer[1];
#ifdef SENDIAN
    switchEndian(l);
#endif
    blk_strm.write(l);
    l = opad_buffer[2];
#ifdef SENDIAN
    switchEndian(l);
#endif
    blk_strm.write(l);
    l = opad_buffer[3];
#ifdef SENDIAN
    switchEndian(l);
#endif
    blk_strm.write(l);
    l = opad_buffer[4];
#ifdef SENDIAN
    switchEndian(l);
#endif
    blk_strm.write(l);
    l = opad_buffer[5];
#ifdef SENDIAN
    switchEndian(l);
#endif
    blk_strm.write(l);
    l = opad_buffer[6];
#ifdef SENDIAN
    switchEndian(l);
#endif
    blk_strm.write(l);
    l = opad_buffer[7];
#ifdef SENDIAN
    switchEndian(l);
#endif
    blk_strm.write(l);
    l = opad_buffer[8];
#ifdef SENDIAN
    switchEndian(l);
#endif
    blk_strm.write(l);
    l = opad_buffer[9];
#ifdef SENDIAN
    switchEndian(l);
#endif
    blk_strm.write(l);
    l = opad_buffer[10];
#ifdef SENDIAN
    switchEndian(l);
#endif
    blk_strm.write(l);
    l = opad_buffer[11];
#ifdef SENDIAN
    switchEndian(l);
#endif
    blk_strm.write(l);
    l = opad_buffer[12];
#ifdef SENDIAN
    switchEndian(l);
#endif
    blk_strm.write(l);
    l = opad_buffer[13];
#ifdef SENDIAN
    switchEndian(l);
#endif
    blk_strm.write(l);
    l = opad_buffer[14];
#ifdef SENDIAN
    switchEndian(l);
#endif
    blk_strm.write(l);
    l = opad_buffer[15];
#ifdef SENDIAN
    switchEndian(l);
#endif
    blk_strm.write(l);

    l = digest_input.range(63, 0);
#ifdef SENDIAN
    switchEndian(l);
#endif
    blk_strm.write(l);
    l = digest_input.range(127, 64);
#ifdef SENDIAN
    switchEndian(l);
#endif
    blk_strm.write(l);
    l = digest_input.range(191, 128);
#ifdef SENDIAN
    switchEndian(l);
#endif
    blk_strm.write(l);
    l = digest_input.range(255, 192);
#ifdef SENDIAN
    switchEndian(l);
#endif
    blk_strm.write(l);
    l = digest_input.range(319, 256);
#ifdef SENDIAN
    switchEndian(l);
#endif
    blk_strm.write(l);
    l = digest_input.range(383, 320);
#ifdef SENDIAN
    switchEndian(l);
#endif
    blk_strm.write(l);

    counter = 6;

    // pad "1"
    {
      blk_strm.write("9223372036854775808");
      counter++;
    }
    // pad "0"
    {
      ap_uint<5> z_rounds;
      if (counter < 15) {
        z_rounds = 14 - counter;
      } else {
        z_rounds = 30 - counter;
      }
      for (ap_uint<5> z = 0; z < z_rounds; z++) {
        blk_strm.write(0);
      }
    }
    // pad "len"
    {
      total_len <<= 3;
      ap_uint<65> L = total_len.range(127, 64);
      L = total_len.range(63, 0);
      L[64] = 1;

      blk_strm.write("0");
      blk_strm.write("18446744073709553024");
    }
  }

} // end preProcessingOneTripForHMAC

template <unsigned int hash_width>
void generateMsgScheduleOneTrip(hls::stream<ap_uint<64 + 1>> &blk_strm,
                                ap_uint<hash_width> &res) {
  static const ap_uint<64> K[80] = {
      0x428a2f98d728ae22UL, 0x7137449123ef65cdUL, 0xb5c0fbcfec4d3b2fUL,
      0xe9b5dba58189dbbcUL, 0x3956c25bf348b538UL, 0x59f111f1b605d019UL,
      0x923f82a4af194f9bUL, 0xab1c5ed5da6d8118UL, 0xd807aa98a3030242UL,
      0x12835b0145706fbeUL, 0x243185be4ee4b28cUL, 0x550c7dc3d5ffb4e2UL,
      0x72be5d74f27b896fUL, 0x80deb1fe3b1696b1UL, 0x9bdc06a725c71235UL,
      0xc19bf174cf692694UL, 0xe49b69c19ef14ad2UL, 0xefbe4786384f25e3UL,
      0x0fc19dc68b8cd5b5UL, 0x240ca1cc77ac9c65UL, 0x2de92c6f592b0275UL,
      0x4a7484aa6ea6e483UL, 0x5cb0a9dcbd41fbd4UL, 0x76f988da831153b5UL,
      0x983e5152ee66dfabUL, 0xa831c66d2db43210UL, 0xb00327c898fb213fUL,
      0xbf597fc7beef0ee4UL, 0xc6e00bf33da88fc2UL, 0xd5a79147930aa725UL,
      0x06ca6351e003826fUL, 0x142929670a0e6e70UL, 0x27b70a8546d22ffcUL,
      0x2e1b21385c26c926UL, 0x4d2c6dfc5ac42aedUL, 0x53380d139d95b3dfUL,
      0x650a73548baf63deUL, 0x766a0abb3c77b2a8UL, 0x81c2c92e47edaee6UL,
      0x92722c851482353bUL, 0xa2bfe8a14cf10364UL, 0xa81a664bbc423001UL,
      0xc24b8b70d0f89791UL, 0xc76c51a30654be30UL, 0xd192e819d6ef5218UL,
      0xd69906245565a910UL, 0xf40e35855771202aUL, 0x106aa07032bbd1b8UL,
      0x19a4c116b8d2d0c8UL, 0x1e376c085141ab53UL, 0x2748774cdf8eeb99UL,
      0x34b0bcb5e19b48a8UL, 0x391c0cb3c5c95a63UL, 0x4ed8aa4ae3418acbUL,
      0x5b9cca4f7763e373UL, 0x682e6ff3d6b2b8a3UL, 0x748f82ee5defb2fcUL,
      0x78a5636f43172f60UL, 0x84c87814a1f0ab72UL, 0x8cc702081a6439ecUL,
      0x90befffa23631e28UL, 0xa4506cebde82bde9UL, 0xbef9a3f7b2c67915UL,
      0xc67178f2e372532bUL, 0xca273eceea26619cUL, 0xd186b8c721c0c207UL,
      0xeada7dd6cde0eb1eUL, 0xf57d4f7fee6ed178UL, 0x06f067aa72176fbaUL,
      0x0a637dc5a2c898a6UL, 0x113f9804bef90daeUL, 0x1b710b35131c471bUL,
      0x28db77f523047d84UL, 0x32caab7b40c72493UL, 0x3c9ebe0a15c9bebcUL,
      0x431d67c49c100d4cUL, 0x4cc5d4becb3e42b6UL, 0x597f299cfc657e2aUL,
      0x5fcb6fab3ad6faecUL, 0x6c44198c4a475817UL,
  };
#pragma HLS array_partition variable = K complete
#pragma HLS bind_storage variable = K type = ROM_1P impl = LUTRAM

  ap_uint<64> H[8];
#pragma HLS array_partition variable = H complete
  if (hash_width == 384) {
    H[0] = 0xcbbb9d5dc1059ed8UL;
    H[1] = 0x629a292a367cd507UL;
    H[2] = 0x9159015a3070dd17UL;
    H[3] = 0x152fecd8f70e5939UL;
    H[4] = 0x67332667ffc00b31UL;
    H[5] = 0x8eb44a8768581511UL;
    H[6] = 0xdb0c2e0d64f98fa7UL;
    H[7] = 0x47b5481dbefa4fa4UL;
  } else if (hash_width == 256) {
    H[0] = 0x22312194FC2BF72CUL;
    H[1] = 0x9F555FA3C84C64C2UL;
    H[2] = 0x2393B86B6F53B151UL;
    H[3] = 0x963877195940EABDUL;
    H[4] = 0x96283EE2A88EFFE3UL;
    H[5] = 0xBE5E1E2553863992UL;
    H[6] = 0x2B0199FC2C85B8AAUL;
    H[7] = 0x0EB72DDC81C52CA2UL;
  } else if (hash_width == 224) {
    H[0] = 0x8C3D37C819544DA2UL;
    H[1] = 0x73E1996689DCD4D6UL;
    H[2] = 0x1DFAB7AE32FF9C82UL;
    H[3] = 0x679DD514582F9FCFUL;
    H[4] = 0x0F6D2B697BD44DA8UL;
    H[5] = 0x77E36F7304C48942UL;
    H[6] = 0x3F9D85A86A1D36C8UL;
    H[7] = 0x1112E6AD91D692A1UL;
  } else {
    H[0] = 0x6a09e667f3bcc908UL;
    H[1] = 0xbb67ae8584caa73bUL;
    H[2] = 0x3c6ef372fe94f82bUL;
    H[3] = 0xa54ff53a5f1d36f1UL;
    H[4] = 0x510e527fade682d1UL;
    H[5] = 0x9b05688c2b3e6c1fUL;
    H[6] = 0x1f83d9abfb41bd6bUL;
    H[7] = 0x5be0cd19137e2179UL;
  }

  ap_uint<8> counter = 0;
  bool run = true;
  ap_uint<64> W[16];
#pragma HLS array_partition variable = W
  ap_uint<64> a, b, c, d, e, f, g, h;
  while (run || counter != 0) {
#pragma HLS pipeline II = 100 // KAMIL
    if (counter == 0) {
      a = H[0];
      b = H[1];
      c = H[2];
      d = H[3];
      e = H[4];
      f = H[5];
      g = H[6];
      h = H[7];
    }

    ap_uint<64> Wt;
    if (counter < 16) {
      ap_uint<65> tmp_Wt = blk_strm.read();
      Wt = tmp_Wt.range(63, 0);
      W[counter] = Wt.range(63, 0);
      if (tmp_Wt[64] == 1) {
        run = false;
      }
    } else {
      Wt = SSIG1<64>(W[14]) + W[9] + SSIG0<64>(W[1]) + W[0];
      for (ap_uint<5> i = 0; i < 15; i++) {
        W[i] = W[i + 1];
      }
      W[15] = Wt;
    }

    ap_uint<64> T1 = h + BSIG1<64>(e) + Ch<64>(e, f, g) + Wt + K[counter];
    ap_uint<64> T2 = BSIG0<64>(a) + Maj<64>(a, b, c);
    h = g;
    g = f;
    f = e;
    e = d + T1;
    d = c;
    c = b;
    b = a;
    a = T1 + T2;

    if (++counter == 80) {
      counter = 0;
      H[0] = a + H[0];
      H[1] = b + H[1];
      H[2] = c + H[2];
      H[3] = d + H[3];
      H[4] = e + H[4];
      H[5] = f + H[5];
      H[6] = g + H[6];
      H[7] = h + H[7];
    }
  }

  ap_uint<512> digest;
LOOP_SHA512_EMIT:
  for (ap_uint<4> i = 0; i < 8; i++) {
#pragma HLS unroll
    ap_uint<64> l = H[i];
    switchEndian(l);
    digest.range(64 * i + 63, 64 * i) = l;
  }
  res = digest.range(hash_width - 1, 0);
} // end generateMsgScheduleOneTrip

void HMAC_SHA384_2in1(ap_uint<2> op_type, ap_uint<64> opad_buffer[16],
                      ap_uint<384> digest_input,
                      hls::stream<ap_uint<512>> &msg_strm,
                      hls::stream<ap_uint<40>> &len_strm,
                      ap_uint<384> &digest_output) {
#pragma HLS inline off
#pragma HLS dataflow
  hls::stream<ap_uint<65>> blk_strm("blk_strm");
#pragma HLS stream variable = blk_strm depth = 128
#pragma HLS resource variable = blk_strm core = FIFO_LUTRAM
  preProcessingOneTripForHMAC(op_type, opad_buffer, digest_input, msg_strm,
                              len_strm, blk_strm);
  generateMsgScheduleOneTrip<384>(blk_strm, digest_output);
}

void convert32To64OneRound(hls::stream<ap_uint<32>> &msg_32_strm,
                           hls::stream<ap_uint<4>> &len_32_strm,
                           hls::stream<ap_uint<64>> &msg_64_strm,
                           hls::stream<ap_uint<5>> &len_64_strm) {
  bool run = true;
  ap_int<64> msg_64 = 0;
  ap_uint<5> len_64 = 0;
  ap_uint<1> half = 0;
  while (run) {
#pragma HLS pipeline II = 1
    ap_uint<4> len_32 = len_32_strm.read();
    ap_uint<32> msg_32 = msg_32_strm.read();

    if (half == 0) {
      msg_64 = msg_32;
      len_64 = len_32.range(2, 0);
    } else {
      msg_64.range(63, 32) = msg_32;
      len_64 += len_32.range(2, 0);
    }

    if (len_32[3] == 1) {
      len_64[4] = 1;
      run = false;
      msg_64_strm.write(msg_64);
      len_64_strm.write(len_64);
    } else if (half == 1) {
      msg_64_strm.write(msg_64);
      len_64_strm.write(len_64);
    }

    half ^= ap_uint<1>(1);
  }
}

void convert32To64(ap_uint<2> opType, hls::stream<ap_uint<32>> &msg_32_strm,
                   hls::stream<ap_uint<4>> &len_32_strm,
                   hls::stream<ap_uint<64>> &msg_64_strm,
                   hls::stream<ap_uint<5>> &len_64_strm) {
  if (opType == 1) {
    convert32To64OneRound(msg_32_strm, len_32_strm, msg_64_strm, len_64_strm);
  }
  if (opType < 2) {
    convert32To64OneRound(msg_32_strm, len_32_strm, msg_64_strm, len_64_strm);
  }
}

} // end namespace internal

/**
 *
 * @brief SHA-384 algorithm with stream input and output.
 *
 * The algorithm reference is : "Secure Hash Standard", which published by NIST
 * in February 2012. The implementation dataflows the pre-processing part and
 * message digest part.
 *
 * @tparam w The bit width of each input message word, default value is 64.
 *
 * @param msg_strm The message being hashed.
 * @param len_strm The message length in byte.
 * @param end_len_strm The flag to signal end of input message stream.
 * @param digest_strm Output digest stream.
 * @param end_digest_strm End flag for output digest stream.
 *
 */

template <unsigned int w>
void sha384(
    // inputs
    hls::stream<ap_uint<w>> &msg_strm, hls::stream<ap_uint<128>> &len_strm,
    hls::stream<bool> &end_len_strm,
    // outputs
    hls::stream<ap_uint<384>> &digest_strm,
    hls::stream<bool> &end_digest_strm) {
  internal::sha512Top<w, 384>(msg_strm, len_strm, end_len_strm, // input streams
                              digest_strm, end_digest_strm); // output streams

} // end sha384

/**
 *
 * @brief SHA-512 algorithm with stream input and output.
 *
 * The algorithm reference is : "Secure Hash Standard", which published by NIST
 * in February 2012. The implementation dataflows the pre-processing part and
 * message digest part.
 *
 * @tparam w The bit width of each input message word, default value is 64.
 *
 * @param msg_strm The message being hashed.
 * @param len_strm The message length in byte.
 * @param end_len_strm The flag to signal end of input message stream.
 * @param digest_strm Output digest stream.
 * @param end_digest_strm End flag for output digest stream.
 *
 */

template <unsigned int w>
void sha512x(
    // inputs
    hls::stream<AXIS_DATA> &input, hls::stream<AXIS_DATA> &output,
    AXIS_DATA &last_input) {
  internal::sha512Top<w, 512>(input,               // input streams
                              output, last_input); // output streams

} // end sha512

/**
 *
 * @brief SHA-512/t algorithm with stream input and output.
 *
 * The algorithm reference is : "Secure Hash Standard", which published by NIST
 * in February 2012. The implementation dataflows the pre-processing part and
 * message digest part.
 *
 * @tparam w The bit width of each input message word, default value is 64.
 * @tparam t The bit width of the digest which depends on specific algorithm,
 * typically is 224 or 256.
 *
 * @param msg_strm The message being hashed.
 * @param len_strm The message length in byte.
 * @param end_len_strm The flag to signal end of input message stream.
 * @param digest_strm Output digest stream.
 * @param end_digest_strm End flag for output digest stream.
 *
 */

template <unsigned int w, unsigned int t>
void sha512_t(
    // inputs
    hls::stream<ap_uint<w>> &msg_strm, hls::stream<bool> &end_len_strm,
    // outputs
    hls::stream<ap_uint<t>> &digest_strm, hls::stream<bool> &end_digest_strm) {
  internal::sha512Top<w, t>(msg_strm, end_len_strm,        // input streams
                            digest_strm, end_digest_strm); // output streams

} // end sha512_t

/**
 * @brief SHA384 + HMAC-SHA384.
 *
 * Each all of HMAC_SHA384 could performce HMAC or SHA384 on one message.
 *
 * @param isHMAC flag to identify actual function, true for HMAC-SHA384
 * processingg, false for SHA384 processing.
 * @param msg_key_strm Stream to input message packs.
 * @param len_strm Stream to input message packs effective length.
 * @param res Digest of SHA384 or HMAC-SHA384
 */
void HMAC_SHA384(hls::stream<AXIS_DATA> &input,
                 hls::stream<AXIS_DATA> &output) {
/**
 * isHMAC: false -> SHA384.
 * Message will be input by msg_key_strm, in multiple 8 bytes pack.
 * Each pack of message will be acommpanied by 5 bits from length from len_strm.
 * Bits[3-0] is used for pure length, ranging from 0 to 8. Bit[4] is used to
 * identify if this is last pack, 1: last, 0: not last.
 *
 * isHMAC: true -> SHA384
 * Both key and message will be input by msg_key_strm, key first, message later.
 * Key should be multiple of 8 byte, no bigger than 128 bytes.
 * Message is similar to SHA384.
 *
 * Please take reference from test cases.
 */
#pragma HLS dataflow
#pragma HLS allocation function instances = internal::HMAC_SHA384_2in1 limit = 1
  ap_uint<64> opad_buffer[16];
  ap_uint<384> first_digest;

  hls::stream<ap_uint<512>> msg_key_strm;
#pragma HLS stream variable = msg_key_strm depth = 32
#pragma HLS resource variable = msg_key_strm core = FIFO_LUTRAM

  hls::stream<ap_uint<40>> len_strm;
#pragma HLS stream variable = len_strm depth = 32
#pragma HLS resource variable = len_strm core = FIFO_LUTRAM
  ap_uint<384> key = "0x3031323334353637383930313233343536373839303132333435363"
                     "73839303132333435363738393031323334353637";

  ap_uint<512> keep;
  ap_uint<6> id;

  injectData(input, msg_key_strm, len_strm, key, keep, id);
  ap_uint<384> res;
  internal::HMAC_SHA384_2in1(1, opad_buffer, first_digest, msg_key_strm,
                             len_strm, res);
  first_digest = res;
  internal::HMAC_SHA384_2in1(2, opad_buffer, first_digest, msg_key_strm,
                             len_strm, res);
  AXIS_DATA r;
  r.tdata = res;
  r.tlast = 1;
  r.tkeep = keep;
  r.tid = id;
  output.write(r);
}
} // namespace security
} // namespace xf

} // namespace AXI
void hmac(hls::stream<AXIS_DATA> &input, hls::stream<AXIS_DATA> &output) {
#pragma HLS INTERFACE ap_ctrl_none port = return // removes handshake
#pragma HLS interface axis register port = input
#pragma HLS interface axis register port = output
#pragma HLS AGGREGATE variable = input bit
#pragma HLS AGGREGATE variable = output bit
  AXI::xf::security::HMAC_SHA384(input, output);
}
