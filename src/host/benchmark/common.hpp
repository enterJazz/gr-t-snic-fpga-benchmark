#ifndef COMMON_HPP
#define COMMON_HPP

namespace benchmark
{
    // output size of SHA256 hash ; input is message hash
    const uint8_t input_msg_hash_size { 32 };
    // output size of HMAC-SHA256 ; attestation is HMAC hash
    const uint8_t output_attestation_hash_size { 32 };

    const size_t device_input_group_id { 0 };
    const size_t device_output_group_id { 1 };
}

#endif
