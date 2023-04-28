#ifndef COMMON_HPP
#define COMMON_HPP

namespace benchmark
{
    // output size of SHA256 hash ; input is message hash
    const uint8_t input_msg_hash_size { 32 };
    // SYM CASE: output size of HMAC-SHA256 ; attestation is HMAC hash
    const uint8_t hmac_sha256_digest_size { 32 };
    // ASYM CASE: EdDSA signature size
    const uint8_t eddsa_signature_size { 64 };

    const size_t device_input_group_id { 0 };
    const size_t device_output_group_id { 1 };

}

#endif
