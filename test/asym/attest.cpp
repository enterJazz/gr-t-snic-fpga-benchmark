#include <catch2/catch.hpp>

// source includes
#include "common.hpp"   // benchmark
#include "kernel.hpp"   // kernel
#include "attest.hpp"   // benchmark::attest
#include "utils.hpp"

// test includes
#include "test_utils.hpp"   // test_utils

// xrt inclues
#include <xrt/xrt_bo.h>
#include <experimental/xrt_xclbin.h>
#include <xrt/xrt_device.h> // device
#include <xrt/xrt_kernel.h>

// system includes
#include <algorithm>    // std::copy
#include <chrono>       // chrono
#include <cstring>   // std::memcpy
#include <filesystem>   // std::filesystem::path

#include <iostream>

namespace bm = benchmark;
namespace tu = test_utils;
namespace attest = benchmark::attest;

SCENARIO( "the asymmetric attest kernel computes valid attestations" )
{
    GIVEN( "the kernel, set to execute in sw_emu" )
    {
        REQUIRE( tu::get_env_var(tu::xcl_emulation_mode_env_var) == tu::xcl_sw_emu_mode );

        std::filesystem::path kernel_xlcbin_path { tu::get_env_var(tu::kernel_asym_attest_path_env_var) };
        REQUIRE( !kernel_xlcbin_path.empty() );

        const kernel::Kernel asym_kernel_type { kernel::asymmetric_attest };

        // xrt specific params
        auto device { xrt::device(0) };


        auto uuid = device.load_xclbin(kernel_xlcbin_path);
        std::cout << kernel_xlcbin_path << std::endl;
        auto krnl = xrt::kernel
        (
            device,
            uuid,
            kernel::kernelToString(asym_kernel_type),
            xrt::kernel::cu_access_mode::exclusive
        );


        WHEN( "we execute the kernel with pre-set inputs" )
        {
            uint8_t preset_msg_hash[bm::input_msg_hash_size] { 0x00 };
            // copy here as msg_hash is const
            std::memcpy(preset_msg_hash, tu::msg_hash, bm::input_msg_hash_size);
            uint8_t attestation_result[bm::eddsa_signature_size] { 0x00 };

            std::chrono::microseconds _bm_res;

            attest::benchmark_attest_kernel
            (
                &_bm_res,
                device,
                krnl,
                asym_kernel_type,   // sym / asym have different inputs
                tu::number_execs,
                // optional params- currently only used for testing
                preset_msg_hash,
                attestation_result
            );

            THEN( "we expect the generated signature to match the pre-computed signature" )
            {
                // TODO: implement logging
                // INFO("OUR SIG: " ); // << std::hex << tu::expected_signature);
                INFO ( "OUR SIG: " << tu::uint8_t_array_to_hex_string(tu::expected_signature, bm::eddsa_signature_size) );
                INFO ( "THEIR SIG: " << tu::uint8_t_array_to_hex_string(attestation_result, bm::eddsa_signature_size) );
                REQUIRE( std::equal(tu::expected_signature, tu::expected_signature + bm::eddsa_signature_size, attestation_result) );
            }

        }

    }

}
