
#include "verify.hpp"

// user includes
#include "common.hpp"
#include "kernel.hpp"   // kernel
#include "utils.hpp"    // utils

// XRT includes
// xrt
#include <xrt/xrt_device.h> // device
#include <xrt/xrt_kernel.h> // kernel, run

// std
#include <stddef.h> // size_t


namespace benchmark::verify
{
    void benchmark_verify_kernel(
        std::chrono::duration<double, std::milli> *result,
        xrt::device device,
        xrt::kernel in_krnl,
        kernel::Kernel krnl_type,   // sym / asym have different inputs
        size_t benchmark_execution_iterations
    )
    {
        // Allocate Buffer in Global Memory
        // share memory between host and FPGA kernel
        //
        // input buffer
        // Match kernel arguments to RTL kernel
        auto bo_in_msg_hash = xrt::bo(
            device,
            input_msg_hash_size,
            in_krnl.group_id(device_input_group_id)
        );

        uint8_t output_attestation_size { (krnl_type == kernel::symmetric_verify) ? hmac_sha256_digest_size : eddsa_signature_size };

        // TODO: is attestation len different
        // NOTE yes is is, 64 (sig) vs 32 (hmac)
        auto bo_in_attestation = xrt::bo(
            device,
            output_attestation_size,
            in_krnl.group_id(device_input_group_id)
        );

        // output buffer
        auto bo_out = xrt::bo(
            device,
            output_attestation_size,
            in_krnl.group_id(device_output_group_id)
        );

        // Map the contents of the buffer object into host memory
        // NOTE: message contents in here
        auto bo_in_msg_hash_map = bo_in_msg_hash.map<uint8_t*>();
        auto bo_in_attestation_map = bo_in_attestation.map<uint8_t*>();
        auto bo_out_map = bo_out.map<uint8_t*>();

        std::fill(
            bo_in_msg_hash_map,
            bo_in_msg_hash_map,
            0
        );
        std::fill(
            bo_in_attestation_map,
            bo_in_attestation_map + output_attestation_size,
            0
        );
        std::fill(bo_out_map, bo_out_map + output_attestation_size, 0);

        // TODO replace with std::genereate ( ... , std::rand )
        utils::populate_input_data(bo_in_msg_hash_map, input_msg_hash_size);
        utils::populate_input_data(bo_in_attestation_map, output_attestation_size);

        // synchronize input buffer data to device global memory
        bo_in_msg_hash.sync(XCL_BO_SYNC_BO_TO_DEVICE);
        bo_in_attestation.sync(XCL_BO_SYNC_BO_TO_DEVICE);

        // Execution of the kernel
        // delay run to benchmark function
        // see https://xilinx.github.io/XRT/2022.1/html/xrt_native_apis.html#other-kernel-apis
        auto run = in_krnl(bo_in_msg_hash, bo_in_attestation, bo_out);
        // bring run into stopped state
        run.wait();

        utils::benchmark_kernel_execution(result, run, benchmark_execution_iterations);
       
    }
}

