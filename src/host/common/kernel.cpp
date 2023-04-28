#include "kernel.hpp"

// std
#include <cstdlib>      // exit, EXIT_FAILURE
#include <filesystem>   // filesystem::path
#include <iostream>     // cerr
#include <string>       // string

const std::string symmetric_attest_str { "SYMMETRIC_ATTEST" };
const std::string symmetric_verify_str { "SYMMETRIC_VERIFY" };
const std::string asymmetric_attest_str { "ASYMMETRIC_ATTEST" };
const std::string asymmetric_verify_str { "ASYMMETRIC_VERIFY" };
const std::string empty_str { "EMPTY" };


namespace kernel
{

    // convert kernel enum to string representation
    // string representation also serves as kernel identifier for execution
    const std::string kernelToString(Kernel kernel)
    {
        switch(kernel)
        {
            case symmetric_attest:  return symmetric_attest_str;
            case symmetric_verify:  return symmetric_verify_str;
            case asymmetric_attest: return asymmetric_attest_str;
            case asymmetric_verify: return asymmetric_verify_str;
            case empty:             return empty_str;
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
        if (kernel_string == symmetric_attest_str)
        {
            return symmetric_attest;
        }
        else if (kernel_string == asymmetric_attest_str)
        {
            return asymmetric_attest;
        }
        else if (kernel_string == symmetric_verify_str)
        {
            return symmetric_verify;
        }
        else if (kernel_string == asymmetric_verify_str)
        {
            return asymmetric_verify;
        }
        else if (kernel_string == empty_str)
        {
            return empty;
        }
        else
        {
            std::cerr << "given kernel string: " << kernel_string
                << " is invalid.\n";
            std::exit(EXIT_FAILURE);
        }
    }
}
