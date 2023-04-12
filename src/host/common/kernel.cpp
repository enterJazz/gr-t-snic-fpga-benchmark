#include "kernel.hpp"

// std
#include <cstdlib>      // exit, EXIT_FAILURE
#include <filesystem>   // filesystem::path
#include <iostream>     // cerr
#include <string>       // string

// macros
#define SYMMETRIC_ATTEST_STR "SYMMETRIC_ATTEST"
#define SYMMETRIC_VERIFY_STR "SYMMETRIC_VERIFY"

namespace kernel
{

    // convert kernel enum to string representation
    // string representation also serves as kernel identifier for execution
    const std::string kernelToString(Kernel kernel)
    {
        switch(kernel)
        {
            case symmetric_attest:  return SYMMETRIC_ATTEST_STR;
            case symmetric_verify:  return SYMMETRIC_VERIFY_STR;
            default:
                std::cerr << "given kernel enum: " << kernel << " is invalid.\n";
                std::exit(EXIT_FAILURE);
        }
    }

    const std::string returnAvailableKernels()
    {
        return std::string {
            "[" + kernelToString(symmetric_attest) + "|"
                + kernelToString(symmetric_verify) + "]"
        };
    }

    const Kernel stringToKernel(const std::string kernel_string)
    {
        if (kernel_string == SYMMETRIC_ATTEST_STR)
        {
            return symmetric_attest;
        }
        else if (kernel_string == SYMMETRIC_VERIFY_STR)
        {
            return symmetric_verify;
        }
        else
        {
            std::cerr << "given kernel string: " << kernel_string
                << " is invalid.\n";
            std::exit(EXIT_FAILURE);
        }
    }
}
