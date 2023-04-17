#ifndef BENCHMARK_HPP
#define BENCHMARK_HPP

// user
#include "kernel.hpp"   // kernel::Kernel

// std
#include <filesystem>   // filesystem::path

namespace benchmark
{
    void benchmark_kernel
    (
        kernel::Kernel target_kernel,
        std::filesystem::path kernel_xlcbin_path,
        size_t benchmark_execution_iterations
    );
}

#endif
