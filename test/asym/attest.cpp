#include <catch2/catch.hpp>

// source includes
#include "kernel.hpp"   // kernel

#include "test_utils.hpp"   // test_utils

#include <xrt/xrt_kernel.h>

namespace tu = test_utils;

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

            THEN( "we receive the expected signature" )
            {

            }

        }

    }

}
