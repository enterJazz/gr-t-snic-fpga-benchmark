#include "test_utils.hpp"

#include <cstdlib>
#include <string>

namespace "test_utils"
{
    std::string get_env_var( std::string const & key ) const
    {
        char * val = getenv( key.c_str() );
        return val == NULL ? std::string("") : std::string(val);
    }
}
