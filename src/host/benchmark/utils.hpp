#ifndef UTILS_HPP
#define UTILS_HPP

// std
#include <stddef.h> // size_t
#include <stdint.h> // uint8_t

namespace benchmark::utils
{

    // populates given array with random data for input
    void populate_input_data(uint8_t *out, size_t size);

}

#endif
