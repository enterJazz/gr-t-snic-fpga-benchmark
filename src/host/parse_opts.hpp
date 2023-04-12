#ifndef PARSE_OPTS_HPP
#define PARSE_OPTS_HPP

#include <string>       // string
#include <string_view>  // string_view
#include <vector>       // vector

namespace ParseOpts
{
    enum Option
    {
        help,
        kernel,
        log_file,
    };

    constexpr std::string_view getOption(const Option opt);


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
