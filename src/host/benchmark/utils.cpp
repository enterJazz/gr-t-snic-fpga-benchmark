
#include "utils.hpp"

// XRT includes
#include <xrt/xrt_kernel.h> // xrt::run

// std
#include <chrono>   // chrono
#include <random>   // mt19937
#include <stddef.h> // size_t
#include <stdint.h> // uint8_t


namespace benchmark::utils
{
    void populate_input_data(uint8_t *out, size_t size)
    {
       	static std::mt19937 mt{ static_cast<unsigned int>(
            std::chrono::steady_clock::now().time_since_epoch().count()
        ) };

        for (size_t i=0 ; i<size ; i++)
        {
            out[i] = mt();
        }
    }


    void benchmark_kernel_execution(
        std::chrono::duration<double, std::milli> *result,
        xrt::run kernel_run)
    {

        auto start = std::chrono::steady_clock::now();

        kernel_run.start();
        kernel_run.wait();

        auto end = std::chrono::steady_clock::now();
        auto diff = end - start;

        *result = diff;
    }

}

