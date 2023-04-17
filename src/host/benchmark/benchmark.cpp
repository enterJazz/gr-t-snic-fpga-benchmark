/**
 * Executes and thereby benchmarks a given FPGA kernel.
 * Writes results to a log file
*/

#include "benchmark.hpp"

// user
#include "kernel.hpp"
#include "utils.hpp"    // utils::populate_input_data

// XRT includes
// see https://xilinx.github.io/XRT/2022.1/html/xrt_native_apis.html
#include <xrt/xrt_bo.h>
#include <experimental/xrt_xclbin.h>
#include <xrt/xrt_device.h>
#include <xrt/xrt_kernel.h>

// System includes
//#include "cmdlineparser.h"
#include <iostream>     // std::cerr
#include <cstdlib>      // std::exit, EXIT_FAILURE
#include <cstring>
#include <stdint.h>     // uint8_t
#include <stdio.h>      // printf
#include <filesystem>   // std::filesystem::path


namespace benchmark
{
    // FPGA device index ; required to open FPGA
    const uint8_t device_index { 0 };
    // output size of SHA256 hash ; input is message hash
    const uint8_t input_msg_hash_size { 32 };
    // output size of HMAC-SHA256 ; attestation is HMAC hash
    const uint8_t output_attestation_hash_size { 32 };

    // IDEA:
    // identify shared point: in kernel::run
    // only # of params change
    //
    // attest(msg) : return attestation
    void benchmark_kernel
    (
        kernel::Kernel target_kernel,
        std::filesystem::path kernel_xlcbin_path
    )
    {

        // access FPGA
        auto device = xrt::device(device_index);
        auto uuid = device.load_xclbin(kernel_xlcbin_path);

        auto krnl = xrt::kernel
        (
            device,
            uuid,
            kernel::kernelToString(target_kernel),
            xrt::kernel::cu_access_mode::exclusive
        );

        // NOTE: the # of buffers could change between funcs
        // Allocate Buffer in Global Memory
        // share memory between host and FPGA kernel
        //
        // input buffer
        // Match kernel arguments to RTL kernel
        auto boIn = xrt::bo(device, input_msg_hash_size, krnl.group_id(0));
        // output buffer
        auto boOut = xrt::bo(device, output_attestation_hash_size, krnl.group_id(1));

        // Map the contents of the buffer object into host memory
        // NOTE: message contents in here
        auto bo0_map = boIn.map<uint8_t*>();
        auto bo1_map = boOut.map<uint8_t*>();
        std::fill(bo0_map, bo0_map + input_msg_hash_size, 0);
        std::fill(bo1_map, bo1_map + output_attestation_hash_size, 0);

        utils::populate_input_data(bo0_map, input_msg_hash_size);

        // synchronize input buffer data to device global memory
        boIn.sync(XCL_BO_SYNC_BO_TO_DEVICE);

        // Execution of the kernel
        // delay run to benchmark function
        // see https://xilinx.github.io/XRT/2022.1/html/xrt_native_apis.html#other-kernel-apis
        auto run = krnl(boIn, boOut);
        // REFACTORME
        run.set_arg(0, boIn);
        run.set_arg(1, boOut);
        // run.wait();

        std::chrono::duration<double, std::milli> result;

        utils::benchmark_kernel_execution(&result, run);
        std::cout << "RESULT: " << result.count() << "\n";

        // get the output from the device
        boOut.sync(XCL_BO_SYNC_BO_FROM_DEVICE);

        // TODO: Validate results
        // if (std::memcmp(bo2_map, bufReference, DATA_SIZE))
        //    throw std::runtime_error("Value read back does not match reference");

        for (int i = 0 ; i < output_attestation_hash_size ; i++)
        {
            std::printf("%x", bo1_map[i]);
        }
        std::cout << "\n";
    }
}
