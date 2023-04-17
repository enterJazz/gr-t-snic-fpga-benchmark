
#include "attest.hpp"

// XRT inclues
#include <xrt/xrt_bo.h>     // xrt::bo
#include <xrt/xrt_kernel.h> // xrt::kernel, xrt::run

namespace benchmark::attest
{
    void load_kernel_run(xrt::run *out_run, const xrt::kernel in_krnl)
    {
        // Allocate Buffer in Global Memory
        // share memory between host and FPGA kernel
        //
        // input buffer
        // Match kernel arguments to RTL kernel
        auto bo_in = xrt::bo(device, input_msg_hash_size, krnl.group_id(0));
        // output buffer
        auto bo_out = xrt::bo(device, output_attestation_hash_size, krnl.group_id(1));

        // Map the contents of the buffer object into host memory
        // NOTE: message contents in here
        auto bo0_map = bo_in.map<uint8_t*>();
        auto bo1_map = bo_out.map<uint8_t*>();
        std::fill(bo0_map, bo0_map + input_msg_hash_size, 0);
        std::fill(bo1_map, bo1_map + output_attestation_hash_size, 0);


    }
}

