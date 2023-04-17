#ifndef ATTEST_HPP
#define ATTEST_HPP

// XRT includes
// xrt
#include <xrt/xrt_device.h> // device
#include <xrt/xrt_kernel.h> // kernel, run

// std
#include <stddef.h> // size_t

namespace benchmark::attest
{
    void benchmark_attest_kernel(
        std::chrono::duration<double, std::milli> *result,
        xrt::device         device,
        const xrt::kernel   in_krnl,
        size_t benchmark_execution_iterations
    );
}

#endif
