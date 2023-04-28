
#include "attest.hpp"

// user includes
#include "common.hpp"
#include "utils.hpp"    // utils

// XRT inclues
// xrt
#include <xrt/xrt_bo.h>     // bo
#include <xrt/xrt_device.h> // device
#include <xrt/xrt_kernel.h> // kernel, run

// std
#include <stdint.h>         // uint8_t

namespace benchmark::attest
{
    void benchmark_attest_kernel
    (
        std::chrono::duration<double, std::milli> *result,
        xrt::device device,
        xrt::kernel in_krnl,
        size_t benchmark_execution_iterations
    )
    {
        // Allocate Buffer in Global Memory
        // share memory between host and FPGA kernel
        //
        // input buffer
        // Match kernel arguments to RTL kernel
        auto bo_in_msg_hash = xrt::bo(device, input_msg_hash_size, in_krnl.group_id(device_input_group_id));
        // output buffer
        auto bo_out = xrt::bo(device, output_attestation_hash_size, in_krnl.group_id(device_output_group_id));

        // Map the contents of the buffer object into host memory
        // NOTE: message contents in here
        auto bo0_map = bo_in_msg_hash.map<uint8_t*>();
        auto bo1_map = bo_out.map<uint8_t*>();
        std::fill(bo0_map, bo0_map + input_msg_hash_size, 0);
        std::fill(bo1_map, bo1_map + output_attestation_hash_size, 0);

        utils::populate_input_data(bo0_map, input_msg_hash_size);

        // synchronize input buffer data to device global memory
        bo_in_msg_hash.sync(XCL_BO_SYNC_BO_TO_DEVICE);

        // Execution of the kernel
        // delay run to benchmark function
        // see https://xilinx.github.io/XRT/2022.1/html/xrt_native_apis.html#other-kernel-apis
        auto run = in_krnl(bo_in_msg_hash, bo_out);
        // bring run into stopped state
        run.wait();

        utils::benchmark_kernel_execution(result, run, benchmark_execution_iterations);
    }
}

