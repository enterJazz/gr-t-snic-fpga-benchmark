
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
#include <chrono>
#include <stddef.h> // size_t


namespace benchmark::verify
{
    void benchmark_verify_kernel(
        std::chrono::microseconds *result,
        xrt::device device,
        xrt::kernel in_krnl,
        kernel::Kernel krnl_type,   // sym / asym have different inputs
        size_t benchmark_execution_iterations,
        // optional params- currently only used for testing
        uint8_t* preset_msg_hash,
        uint8_t* preset_msg_attestation,
        uint8_t* preset_pubkey,
        uint8_t* attestation_result
    )
    {
        uint8_t output_attestation_size { (krnl_type == kernel::symmetric_verify) ? hmac_sha256_digest_size : eddsa_signature_size };

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
        // TODO: is attestation len different
        // NOTE yes is is, 64 (sig) vs 32 (hmac)
        auto bo_in_attestation = xrt::bo(
            device,
            output_attestation_size,
            in_krnl.group_id(device_input_group_id)
        );
        auto bo_in_pubkey = xrt::bo(
            device,
            eddsa_pubkey_size,
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
        auto bo_in_pubkey_map = bo_in_pubkey.map<uint8_t*>();
        auto bo_out_map = bo_out.map<uint8_t*>();

        // TODO: DRY, do as func

        utils::copy_else_fill(bo_in_msg_hash_map, input_msg_hash_size, preset_msg_hash);
        utils::copy_else_fill(bo_in_attestation_map, output_attestation_size, preset_msg_attestation);
        utils::copy_else_fill(bo_in_pubkey_map, eddsa_pubkey_size, preset_pubkey);

        std::fill(bo_out_map, bo_out_map + output_attestation_size, 0);

        // TODO replace with std::genereate ( ... , std::rand )

        // synchronize input buffer data to device global memory
        bo_in_msg_hash.sync(XCL_BO_SYNC_BO_TO_DEVICE);
        bo_in_attestation.sync(XCL_BO_SYNC_BO_TO_DEVICE);
        bo_in_pubkey.sync(XCL_BO_SYNC_BO_TO_DEVICE);

        // Execution of the kernel
        // delay run to benchmark function
        // see https://xilinx.github.io/XRT/2022.1/html/xrt_native_apis.html#other-kernel-apis
        auto run { (krnl_type == kernel::symmetric_verify) ? in_krnl(bo_in_msg_hash, bo_in_attestation, bo_out) : in_krnl(bo_in_msg_hash, bo_in_attestation, bo_in_pubkey, bo_out) };
        // bring run into stopped state
        run.wait();

        utils::benchmark_kernel_execution(result, run, benchmark_execution_iterations);
       
        if (attestation_result)
        {
            // synchronize output device global memory to buffer data
            bo_out.sync(XCL_BO_SYNC_BO_TO_DEVICE);
            std::copy(attestation_result, attestation_result + output_attestation_size, bo_out_map);
        }
    }
}

