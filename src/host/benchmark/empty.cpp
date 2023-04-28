
#include "empty.hpp"

#include "common.hpp"

// user includes
#include "utils.hpp"    // utils

// XRT includes
// xrt
#include <xrt/xrt_device.h> // device
#include <xrt/xrt_kernel.h> // kernel, run

// std
#include <chrono>
#include <stddef.h>


namespace benchmark::empty
{
    void benchmark_empty_kernel(
        std::chrono::microseconds *result,
        xrt::device device,
        xrt::kernel in_krnl,
        size_t benchmark_execution_iterations
    )
    {
        // Buffers allocated contain no input-
        // only used as required to instantiate kernel `in_krnl(Args&& ...)`
        //
        // Allocate Buffer in Global Memory
        // share memory between host and FPGA kernel
        //
        // input buffer
        // Match kernel arguments to RTL kernel
        auto bo_in = xrt::bo(
                device,
                input_msg_hash_size,
                in_krnl.group_id(device_input_group_id)
        );

        // Map the contents of the buffer object into host memory
        auto bo_in_map = bo_in.map<uint8_t*>();
        std::fill(
            bo_in_map,
            bo_in_map + input_msg_hash_size,
            0
        );

        utils::populate_input_data(bo_in_map, input_msg_hash_size);

        // synchronize input buffer data to device global memory
        bo_in.sync(XCL_BO_SYNC_BO_TO_DEVICE);

        // Execution of the kernel
        // delay run to benchmark function
        // see https://xilinx.github.io/XRT/2022.1/html/xrt_native_apis.html#other-kernel-apis
        auto run = in_krnl(bo_in);
        // bring run into stopped state
        run.wait();

        utils::benchmark_kernel_execution(result, run, benchmark_execution_iterations);
    }
}
