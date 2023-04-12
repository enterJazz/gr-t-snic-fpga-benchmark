// std
#include <algorithm>    // find
#include <cstdlib>      // exit, EXIT_FAILURE
#include <iostream>        // cout, cerr
#include <string>       // string
#include <string_view>  // string_view
#include <vector>       // vector

namespace OptParse
{
    enum Option
    {
        help,
        kernel,
        log_file,
    };

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

    // https://stackoverflow.com/a/868894
    class InputParser
    {
        public:
            InputParser (int &argc, char **argv)
            {
                for (int i=1; i < argc; ++i)
                    this->tokens.push_back(std::string(argv[i]));
            }

            const std::string& getCmdOption(const Option enum_option) const
            {
                const std::string &option { getOption(enum_option) };
                std::vector<std::string>::const_iterator itr;
                itr =  std::find(this->tokens.begin(), this->tokens.end(), option);
                if (itr != this->tokens.end() && ++itr != this->tokens.end())
                {
                    return *itr;
                }
                static const std::string empty_string("");
                return empty_string;
            }

            bool cmdOptionExists(const Option enum_option) const
            {
                const std::string &option { getOption(enum_option) };
                return std::find(this->tokens.begin(), this->tokens.end(), option)
                       != this->tokens.end();
            }

        private:
            std::vector <std::string> tokens;
    };
}




int main(int argc, char **argv)
{
    OptParse::InputParser input(argc, argv);
    if (input.cmdOptionExists(OptParse::help))
    {
        // Do stuff
    }
    const std::string &filename = input.getCmdOption(OptParse::kernel);
    if (!filename.empty())
    {
        // Do interesting things ...
    }
    return 0;
}

