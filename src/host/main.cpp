// user
// ParseOpts
#include "parse_opts.hpp"   // InputParser, Options

// std
#include <algorithm>    // find
#include <cstdlib>      // exit, EXIT_FAILURE
#include <iostream>     // cout, cerr
#include <string>       // string
#include <string_view>  // string_view
#include <vector>       // vector


int main(int argc, char **argv)
{
    ParseOpts::InputParser input(argc, argv);
    if (input.cmdOptionExists(ParseOpts::help))
    {
        // Do stuff
    }
    const std::string &filename = input.getCmdOption(ParseOpts::kernel);
    if (!filename.empty())
    {
        // Do interesting things ...
    }
    return 0;
}

