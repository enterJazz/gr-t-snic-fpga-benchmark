#ifndef PARSE_OPTS_HPP
#define PARSE_OPTS_HPP

// std
#include <iostream>
#include <string>       // string
#include <string_view>  // string_view
#include <vector>       // vector

namespace ParseOpts
{
    enum Option
    {
        help,
        kernel,
        kernel_xlcbin,
        num_iterations,
    };

    constexpr std::string_view getOption(const Option opt)
    {
        switch (opt)
        {
            case help:              return "-h";
            case kernel:            return "-k";
            case kernel_xlcbin:     return "-x";
            case num_iterations:    return "-n";
            default:
                std::cerr << "unknown opt: " << opt << "\n";
                std::exit(EXIT_FAILURE);
        }
    }

    // https://stackoverflow.com/a/868894
    class InputParser
    {
        public:
            InputParser (int &argc, char **argv);

            const std::string& getCmdOption(const Option enum_option) const;
            

            bool cmdOptionExists(const Option enum_option) const;

        private:
            std::vector <std::string> tokens;
    };

}

#endif
