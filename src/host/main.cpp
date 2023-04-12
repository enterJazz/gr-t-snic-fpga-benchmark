// user
// ParseOpts
#include "parse_opts.hpp"   // InputParser, Options

// std
#include <cstdlib>          // exit, EXIT_SUCCESS, EXIT_FAILURE
#include <iostream>         // cout, cerr
#include <string>           // string


int main(int argc, char **argv)
{
    // parse command line args
    const std::string binary_name { argv[0] };

    ParseOpts::InputParser input(argc, argv);
    if (input.cmdOptionExists(ParseOpts::help) || argc == 1)
    {
        std::cout << binary_name << " "
            << ParseOpts::getOption(ParseOpts::kernel) << " [KERNEL_NAME] "
            << ParseOpts::getOption(ParseOpts::log_file)
            << " [LOG_FILE_PATH]\n";
        std::exit(EXIT_SUCCESS);
    }

    const std::string kernel_string = input.getCmdOption(ParseOpts::kernel);
    if (!kernel_string.empty())
    {
        // parse into kernel
    }

    const std::string &filename = input.getCmdOption(ParseOpts::log_file);
    if (filename.empty())
    {
        std::cerr << "-f [LOG_FILE_PATH] required\n";
        std::exit(EXIT_FAILURE);
    }

    return 0;
}
