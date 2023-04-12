/*
 * Enumerates the benchmark kernel functions.
 */

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

    const Kernel stringToKernel(const std::string kernel_str);

    const std::filesystem::path getPathToKernelXlcbin(Kernel kernel);
}


