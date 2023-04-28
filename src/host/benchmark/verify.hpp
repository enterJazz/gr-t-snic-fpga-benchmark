#ifndef VERIFY_HPP
#define VERIFY_HPP

#include "kernel.hpp"

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
    );

}

#endif
