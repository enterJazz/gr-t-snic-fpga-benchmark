#ifndef VERIFY_HPP
#define VERIFY_HPP

#include "kernel.hpp"

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
        uint8_t* preset_msg_hash=nullptr,
        uint8_t* preset_msg_attestation=nullptr,
        uint8_t* preset_pubkey=nullptr,
        uint8_t* attestation_result=nullptr
    );

}

#endif
