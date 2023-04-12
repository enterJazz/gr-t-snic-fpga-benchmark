/*
 * Enumerates the benchmark kernel functions.
 */

#ifndef KERNEL_HPP
#define KERNEL_HPP

// std
#include <filesystem>   // filesystem::path
#include <string>       // string

namespace kernel
{
    enum Kernel
    {
        symmetric_attest,
        symmetric_verify
    };

    const std::string returnAvailableKernels();

    const std::string kernelToString(Kernel kernel);
    const Kernel stringToKernel(const std::string kernel_str);
}

#endif
