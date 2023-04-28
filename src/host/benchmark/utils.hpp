#ifndef UTILS_HPP
#define UTILS_HPP

// XRT includes
#include <xrt/xrt_kernel.h> // xrt::run

// std
#include <chrono>       // chrono
#include <stddef.h>     // size_t
#include <stdint.h>     // uint8_t

namespace benchmark::utils
{

    // populates given array with random data for input
    void populate_input_data(uint8_t *out, size_t size);

    void benchmark_kernel_execution
    (
        std::chrono::microseconds *result,
        xrt::run kernel_run,
        size_t benchmark_execution_iterations
    );

}

#endif
