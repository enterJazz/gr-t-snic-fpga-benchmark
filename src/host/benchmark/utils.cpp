
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


    void benchmark_kernel_execution
    (
        std::chrono::microseconds *result,
        xrt::run kernel_run,
        size_t benchmark_execution_iterations
    )
    {

        // approach: execute kernel `benchmark_execution_iterations` times,
        // sum the execution time for each iteration,
        // divide by number of executions to obtain average execution time
        // as result
        std::chrono::microseconds total_exec_dur { std::chrono::microseconds::zero() };

        for (size_t i_exec = 0; i_exec < benchmark_execution_iterations ; i_exec++)
        {
            std::chrono::steady_clock::time_point start { std::chrono::steady_clock::now() };

            kernel_run.start();
            kernel_run.wait();

            std::chrono::steady_clock::time_point end { std::chrono::steady_clock::now() };
            std::chrono::microseconds diff { std::chrono::duration_cast<std::chrono::microseconds>(end - start) };

            total_exec_dur += diff;
        }

        std::chrono::microseconds avg_exec_dur = total_exec_dur / benchmark_execution_iterations;

        *result = avg_exec_dur;
    }

}

