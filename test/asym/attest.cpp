#include <catch2/catch.hpp>

// source includes
#include "common.hpp"   // benchmark
#include "kernel.hpp"   // kernel
#include "attest.hpp"   // benchmark::attest

// test includes
#include "test_utils.hpp"   // test_utils

// xrt inclues
#include <xrt/xrt_kernel.h>

// system includes
#include <algorithm>    // std::copy

namespace bm = benchmark;
namespace tu = test_utils;
namespace attest = benchmark::attest;

SCENARIO( "the asymmetric attest kernel computes valid attestations" )
{
    GIVEN( "the kernel, set to execute in sw_emu" )
    {
        const std::string kernel_xlcbin_path { tu::get_env_var(tu::kernel_asym_attest_path_env_var) };
        REQUIRE( !kernel_path.empty() );
        const kernel::Kernel asym_kernel_type { kernel::asymmetric_attest };

        auto uuid = tu::device.load_xclbin(kernel_xlcbin_path);
        auto krnl = xrt::kernel
        (
            tu::device,
            uuid,
            kernel::kernelToString(target_kernel),
            xrt::kernel::cu_access_mode::exclusive
        );


        WHEN( "we execute the kernel with pre-set inputs" )
        {

            uint8_t preset_msg_hash[bm::input_msg_hash_size] { 0x00 };
            // copy here as msg_hash is const
            std::copy(preset_msg_hash, preset_msg_hash + bm::input_msg_hash_size, msg_hash);
            uint8_t attestation_result[bm::eddsa_signature_size] { 0x00 };

            attest::benchmark_attest_kernel
            (
                nullptr,
                tu::device,
                krnl,
                asym_kernel_type,   // sym / asym have different inputs
                tu::number_execs,
                // optional params- currently only used for testing
                preset_msg_hash,
                attestation_result
            )

            THEN( "we expect the generated signature to match the pre-computed signature" )
            {

            }

        }

    }

}
