#ifndef TEST_UTILS_HPP
#define TEST_UTILS_HPP


#include <xrt/xrt_device.h> // device

#include <string>
#include <stdint.h>

namespace "test_utils"
{
    // test params
    const uint8_t msg_hash[] {0x3f, 0xc9, 0xb6, 0x89, 0x45, 0x9d, 0x73, 0x8f,
                             0x8c, 0x88, 0xa3, 0xa4, 0x8a, 0xa9, 0xe3, 0x35,
                             0x42, 0x01, 0x6b, 0x7a, 0x40, 0x52, 0xe0, 0x01,
                             0xaa, 0x53, 0x6f, 0xca, 0x74, 0x81, 0x3c, 0xb0};


    // kernel path env vars
    const std::string kernel_asym_attest_path_env_var { "KERNEL_ASYM_ATTEST_XCLBIN" };
    const std::string kernel_asym_verify_path_env_var { "KERNEL_ASYM_VERIFY_XCLBIN" };
    const std::string kernel_sym_attest_path_env_var { "KERNEL_SYM_ATTEST_XCLBIN" };
    const std::string kernel_sym_verify_path_env_var { "KERNEL_SYM_VERIFY_XCLBIN" };

    const uint8_t number_execs { 1 };

    // xrt specific params
    auto device { xrt::device(0) };

    std::string get_env_var( std::string const & key );
}

#endif
