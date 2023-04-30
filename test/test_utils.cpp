#include "test_utils.hpp"

#include <cstdlib>
#include <iomanip>
#include <sstream>
#include <stddef.h>
#include <string>

namespace test_utils
{
    std::string get_env_var( std::string const & key )
    {
        char * val = getenv( key.c_str() );
        return val == NULL ? std::string("") : std::string(val);
    }


    std::string uint8_t_array_to_hex_string(const uint8_t* data, size_t size)
    {
        std::stringstream stream;
        stream << std::hex << std::setfill('0');

        for (size_t i = 0; i < size; ++i) {
            stream << std::setw(2) << static_cast<int>(data[i]);
        }

        return stream.str();
    }
}
