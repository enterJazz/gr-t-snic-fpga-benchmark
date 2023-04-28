#ifndef ATTEST_HPP
#define ATTEST_HPP

#include "kernel.hpp"

// XRT includes
// xrt
#include <xrt/xrt_device.h> // device
#include <xrt/xrt_kernel.h> // kernel, run

// std
#include <chrono>
#include <stddef.h> // size_t

namespace benchmark::attest
{
    void benchmark_attest_kernel(
        std::chrono::microseconds *result,
        xrt::device         device,
        const xrt::kernel   in_krnl,
        kernel::Kernel krnl_type,   // sym / asym have different inputs
        size_t benchmark_execution_iterations
    );
}

#endif
