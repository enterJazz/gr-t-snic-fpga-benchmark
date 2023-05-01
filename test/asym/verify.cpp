#include <catch2/catch.hpp>

// source includes
#include "common.hpp"   // benchmark
#include "kernel.hpp"   // kernel
#include "verify.hpp"   // benchmark::verify
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
#include <cstring>      // std::memcpy
#include <filesystem>   // std::filesystem::path

#include <iostream>

namespace bm = benchmark;
namespace tu = test_utils;
namespace verify = benchmark::verify;

SCENARIO( "the asymmetric verify kernel distinguised in-/valid attestations" )
{
    GIVEN( "the kernel, set to execute in sw_emu" )
    {
        REQUIRE( tu::get_env_var(tu::xcl_emulation_mode_env_var) == tu::xcl_sw_emu_mode );

        std::filesystem::path kernel_xlcbin_path { tu::get_env_var(tu::kernel_asym_verify_path_env_var) };
        REQUIRE( !kernel_xlcbin_path.empty() );

        const kernel::Kernel asym_kernel_type { kernel::asymmetric_verify };

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
            // prepare inputs
            // copy as source is const
            uint8_t preset_msg_hash[bm::input_msg_hash_size] { 0x00 };
            std::memcpy(preset_msg_hash, tu::msg_hash, bm::input_msg_hash_size);
            uint8_t preset_attestation_result[bm::eddsa_signature_size] { 0x00 };
            std::memcpy(preset_attestation_result, tu::expected_signature, bm::eddsa_signature_size);
            uint8_t preset_pubkey[bm::eddsa_pubkey_size] { 0x00 };
            std::memcpy(preset_pubkey, tu::pubkey, bm::eddsa_pubkey_size);
            uint8_t attestation_result[1] { 0x0 };

            std::chrono::microseconds _bm_res;

            verify::benchmark_verify_kernel
            (
                &_bm_res,
                device,
                krnl,
                asym_kernel_type,   // sym / asym have different inputs
                tu::number_execs,
                // optional params- currently only used for testing
                preset_msg_hash,
                preset_attestation_result,
                preset_pubkey,
                attestation_result
            );

            THEN( "we expect the generated signature to match the pre-computed signature" )
            {
                REQUIRE( attestation_result[0] == true );
            }

        }

        WHEN( "we execute the kernel with a false signature" )
        {
            // prepare inputs
            // copy as source is const
            uint8_t preset_msg_hash[bm::input_msg_hash_size] { 0x00 };
            std::memcpy(preset_msg_hash, tu::msg_hash, bm::input_msg_hash_size);

            // preset_attestation_result is only 0s (incorrect)
            uint8_t preset_attestation_result[bm::eddsa_signature_size] { 0x00 };

            uint8_t preset_pubkey[bm::eddsa_pubkey_size] { 0x00 };
            std::memcpy(preset_pubkey, tu::pubkey, bm::eddsa_pubkey_size);
            uint8_t attestation_result[1] { 0x0 };

            std::chrono::microseconds _bm_res;

            verify::benchmark_verify_kernel
            (
                &_bm_res,
                device,
                krnl,
                asym_kernel_type,   // sym / asym have different inputs
                tu::number_execs,
                // optional params- currently only used for testing
                preset_msg_hash,
                preset_attestation_result,
                preset_pubkey,
                attestation_result
            );

            THEN( "we expect to get a failed attestation result" )
            {
                REQUIRE( attestation_result[0] == false );
            }
        }
    }
}
