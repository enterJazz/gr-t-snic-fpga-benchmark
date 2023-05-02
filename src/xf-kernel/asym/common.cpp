
extern "C"
{
#include "common.h"

#include <stdint.h> // uint8_t
#include <cstring>  // memcpy


    namespace common::asym
    {
        void prepare_signature_input
        (
            uint8_t out_input[sign_input_len],
            uint8_t msg_hash[msg_hash_len],
            uint8_t counter_byte_array[counter_len],
            uint32_t counter
        )
        {
            // store counter as byte repr to give as input to signature
            std::memcpy(counter_byte_array, &counter, counter_len);
            
            // concat arrays
            std::memcpy(out_input, msg_hash, msg_hash_len);
            std::memcpy(out_input + msg_hash_len, counter_byte_array, counter_len);
        }

        //template <int W>
//        void print(ap_uint<W> a)
//        {
//            for (int i = 0; i < W; ++i)
//            {
//                if (i % 8 == 0)
//                {
//                    std::cout << " ";
//                }
//
//                std::cout << a[i];
//            }
//
//            std::cout << std::endl;
//        }
    }
}
