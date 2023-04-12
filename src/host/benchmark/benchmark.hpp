// user
#include "kernel.hpp"   // kernel::Kernel

// std
#include <filesystem>   // filesystem::path

namespace benchmark
{
    void benchmark_kernel
    (
        kernel::Kernel target_kernel,
        std::filesystem::path log_file_path
    );
}
