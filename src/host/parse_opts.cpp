#include "parse_opts.hpp"

// std
#include <algorithm>    // find
#include <cstdlib>      // exit, EXIT_FAILURE
#include <iostream>     // cout, cerr
#include <string>       // string
#include <string_view>  // string_view
#include <vector>       // vector


namespace ParseOpts
{
    constexpr std::string_view getOption(const Option opt)
    {
        switch (opt)
        {
            case help:      return "-h";
            case kernel:    return "-k";
            case log_file:  return "-f";
            default:
                std::cerr << "unknown opt: " << opt << "\n";
                std::exit(EXIT_FAILURE);
        }
    }

    InputParser::InputParser (int &argc, char **argv)
    {
        for (int i=1; i < argc; ++i)
            this->tokens.push_back(std::string(argv[i]));
    }

    const std::string& InputParser::getCmdOption(const Option enum_option) const
    {
        std::string option{ getOption(enum_option) };
        std::vector<std::string>::const_iterator itr;
        itr =  std::find(this->tokens.begin(), this->tokens.end(), option);
        if (itr != this->tokens.end() && ++itr != this->tokens.end())
        {
            return *itr;
        }
        static const std::string empty_string("");
        return empty_string;
    }

    bool InputParser::cmdOptionExists(const Option enum_option) const
    {
        const std::string option { getOption(enum_option) };
        return std::find(this->tokens.begin(), this->tokens.end(), option)
               != this->tokens.end();
    }
}
