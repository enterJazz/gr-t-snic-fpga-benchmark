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
    [[maybe_unused]] kernel::Kernel kernel_enum;
    [[maybe_unused]] std::filesystem::path log_file_path;

    ParseOpts::InputParser input(argc, argv);
    if (input.cmdOptionExists(ParseOpts::help) || argc == 1)
    {
        std::cout << binary_name << " "
            << ParseOpts::getOption(ParseOpts::kernel) << " "
            << kernel::returnAvailableKernels() << " "
            << ParseOpts::getOption(ParseOpts::log_file)
            << " [LOG_FILE_PATH]\n";
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

    const std::string log_file_path_string = input.getCmdOption(ParseOpts::log_file);
    if (!log_file_path_string.empty())
    {
        log_file_path = log_file_path_string;
    }
    else
    {
        std::cerr << ParseOpts::getOption(ParseOpts::log_file)
            << " [LOG_FILE_PATH] required\n";
        std::exit(EXIT_FAILURE);
    }

    benchmark::benchmark_kernel
    (
        kernel_enum,
        log_file_path   
    );

    return 0;
}
