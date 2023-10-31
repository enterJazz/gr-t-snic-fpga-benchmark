
#include "rsa.hpp"

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
#include <iostream>

namespace benchmark::rsa
{
    void benchmark_rsa_kernel(
        std::chrono::microseconds& result,
        xrt::device device,
        xrt::kernel in_krnl,
        size_t benchmark_execution_iterations
    )
    {
	    uint8_t output_attestation_size = 64;
	    uint8_t preset_msg_hash[32] = {0x0};
	    uint8_t attestation_result[64] = {0x0};
#if 1
  // Allocate Buffer in Global Memory
  // share memory between host and FPGA kernel
  //
  // input buffer
  // Match kernel arguments to RTL kernel
  auto bo_in_msg_hash = xrt::bo(device, input_msg_hash_size,
                                in_krnl.group_id(device_input_group_id));
  // output buffer
  auto bo_out = xrt::bo(device, output_attestation_size,
                        in_krnl.group_id(device_output_group_id));

  // Map the contents of the buffer object into host memory
  // NOTE: message contents in here
  auto bo0_map = bo_in_msg_hash.map<uint8_t *>();
  auto bo1_map = bo_out.map<uint8_t *>();

  utils::copy_else_fill(bo0_map, input_msg_hash_size, preset_msg_hash);

  std::fill(bo1_map, bo1_map + output_attestation_size, 0);

  // synchronize input buffer data to device global memory
  bo_in_msg_hash.sync(XCL_BO_SYNC_BO_TO_DEVICE);

  // Execution of the kernel
  // delay run to benchmark function
  // see
  // https://xilinx.github.io/XRT/2022.1/html/xrt_native_apis.html#other-kernel-apis
  auto run = in_krnl(bo_in_msg_hash, bo_out);
  run.wait();
    bo_out.sync(XCL_BO_SYNC_BO_TO_DEVICE);
    std::copy(bo1_map, bo1_map + output_attestation_size, attestation_result);
	std::cout << "benchmark_execution_iterations=" << benchmark_execution_iterations << "\n";
  // perform actual benchmarking
  utils::benchmark_kernel_execution(result, run,
                                    benchmark_execution_iterations);
#endif
#if 0
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

	std::cout << __PRETTY_FUNCTION__ << "\n";
        // Map the contents of the buffer object into host memory
        auto bo_in_map = bo_in.map<uint8_t*>();
        std::fill(
            bo_in_map,
            bo_in_map + input_msg_hash_size,
            0
        );

	std::cout << __PRETTY_FUNCTION__ << "\n";
        utils::populate_input_data(bo_in_map, input_msg_hash_size);

        // synchronize input buffer data to device global memory
        bo_in.sync(XCL_BO_SYNC_BO_TO_DEVICE);

        // Execution of the kernel
        // delay run to benchmark function
        // see https://xilinx.github.io/XRT/2022.1/html/xrt_native_apis.html#other-kernel-apis
        auto run = in_krnl(bo_in);
        // bring run into stopped state
        run.wait();

	std::cout << __PRETTY_FUNCTION__ << "\n";
        utils::benchmark_kernel_execution(result, run, benchmark_execution_iterations);
#endif
    }
}
