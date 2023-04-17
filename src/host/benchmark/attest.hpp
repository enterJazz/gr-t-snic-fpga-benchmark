#ifndef ATTEST_HPP
#define ATTEST_HPP

// XRT includes
#include <xrt/xrt_kernel.h> // xrt::kernel, xrt::run

namespace benchmark::attest
{
    void load_kernel_run(xrt::run *out_run, const xrt::kernel in_krnl);
}

#endif
