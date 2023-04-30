

#include <stdint.h>
#include <iostream>

extern "C" {
#include <monocypher.h>
int main() {

    uint8_t eddsa_seed[] {
        0x8a, 0x6a, 0x2f, 0xdd, 0x00, 0xbe, 0x41, 0x24,
        0xc6, 0x28, 0x82, 0xfe, 0xb0, 0x1b, 0x06, 0xdf, 
        0x8e, 0x03, 0x3f, 0xb3, 0x47, 0x16, 0x1c, 0xa3,
        0x65, 0x6c, 0xb1, 0x75, 0xb4, 0x77, 0x91, 0x88, 
    };

    const uint8_t msg_hash[] {0x3f, 0xc9, 0xb6, 0x89, 0x45, 0x9d, 0x73, 0x8f,
                             0x8c, 0x88, 0xa3, 0xa4, 0x8a, 0xa9, 0xe3, 0x35,
                             0x42, 0x01, 0x6b, 0x7a, 0x40, 0x52, 0xe0, 0x01,
                             0xaa, 0x53, 0x6f, 0xca, 0x74, 0x81, 0x3c, 0xb0};


    uint8_t private_key[64] { 0x00 };
    uint8_t public_key[32] { 0x00 };

    crypto_eddsa_key_pair(private_key, public_key, eddsa_seed);


    std::cout << "PUBKEY: "  << std::endl;
    for (int i = 0; i<32; i++) {
        // std::cout << std::hex << std::setfill(' << std::endl;
        printf("0x%x, ", public_key[i]);
    }

    std::cout << std::endl;

    std::cout << "PRIVKEY: " << std::endl;

    for (int i = 0; i<64; i++) {
        // std::cout << std::hex << std::setfill(' << std::endl;
        printf("0x%x, ", private_key[i]);
    }

    std::cout << std::endl;

    std::cout << "SIGNATURE: " << std::endl;
    uint8_t signature[64] { 0x0 };

    crypto_eddsa_sign(signature, private_key, msg_hash, 64);

    for (int i = 0; i<64; i++) {
        // std::cout << std::hex << std::setfill(' << std::endl;
        printf("0x%x, ", signature[i]);
    }

    std::cout << std::endl;
}

}
