#ifndef EMPTY_HPP
#define EMPTY_HPP

// XRT includes
// xrt
#include <xrt/xrt_device.h> // device
#include <xrt/xrt_kernel.h> // kernel, run

// std
#include <chrono>   // chrono
#include <stddef.h> // size_t


namespace benchmark::empty
{
    void benchmark_empty_kernel(
        std::chrono::microseconds *result,
        xrt::device device,
        xrt::kernel in_krnl,
        size_t benchmark_execution_iterations
    );
}

#endif
