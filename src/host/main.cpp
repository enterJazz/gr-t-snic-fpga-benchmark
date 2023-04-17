/*
 * Benchmarks specific FPGA kernels.
 */

// user
// ParseOpts
#include "parse_opts.hpp"   // ParseOpts::InputParser, ParseOpts::Options
#include "kernel.hpp"       // kernel::Kernel, kernel::stringToKernel
#include "benchmark.hpp"

// std
#include <cstdlib>          // exit, EXIT_SUCCESS, EXIT_FAILURE
#include <filesystem>       // filesystem::path
#include <iostream>         // cout, cerr
#include <string>           // string


int main(int argc, char **argv)
{
    // parse command line args
    const std::string binary_name { argv[0] };
    // input args for benchmark function ; parsed in this function
    kernel::Kernel kernel_enum;
    std::filesystem::path kernel_xlcbin_path;

    ParseOpts::InputParser input(argc, argv);
    if (input.cmdOptionExists(ParseOpts::help) || argc == 1)
    {
        std::cout << binary_name << " "
            << ParseOpts::getOption(ParseOpts::kernel) << " "
            << kernel::returnAvailableKernels() << " "
            << ParseOpts::getOption(ParseOpts::kernel_xlcbin) << " "
            << " [KERNEL_XLCBIN_PATH] "
            << "\n";
        std::exit(EXIT_SUCCESS);
    }

    const std::string kernel_string = input.getCmdOption(ParseOpts::kernel);
    if (!kernel_string.empty())
    {
        // parse into kernel enum
        kernel_enum = kernel::stringToKernel(kernel_string);
    }
    else
    {
        std::cerr << ParseOpts::getOption(ParseOpts::kernel)
            << " [KERNEL] required\n";
        std::exit(EXIT_FAILURE);
    }

    const std::string kernel_xlcbin_path_string = input.getCmdOption(ParseOpts::kernel_xlcbin);
    if (!kernel_xlcbin_path_string.empty())
    {
        kernel_xlcbin_path = std::filesystem::path{ kernel_xlcbin_path_string };
    }
    else
    {
        std::cerr << ParseOpts::getOption(ParseOpts::kernel_xlcbin)
            << " [KERNEL_XLCBIN_PATH] required\n";
        std::exit(EXIT_FAILURE);
    }


    benchmark::benchmark_kernel
    (
        kernel_enum,
        kernel_xlcbin_path
    );

    std::exit(EXIT_SUCCESS);
}
