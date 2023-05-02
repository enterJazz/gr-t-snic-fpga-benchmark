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
        symmetric_verify,
        asymmetric_attest,
        asymmetric_verify,
        xf_asymmetric_attest,
        empty
    };

    const std::string returnAvailableKernels();

    const std::string kernelToString(Kernel kernel);
    const Kernel stringToKernel(const std::string kernel_str);
}

#endif
