/**
 * Executes and thereby benchmarks a given FPGA kernel.
 * Writes results to a log file
 */

#include "benchmark.hpp"

// user
#include "attest.hpp" // attest
#include "common.hpp" // input_msg_hash_size, output_attestation_hash_size
#include "empty.hpp"  // empty
#include "kernel.hpp" // kernel
#include "utils.hpp"  // utils
#include "verify.hpp" // verify
#include "rsa.hpp" // rsa

// XRT includes
// see https://xilinx.github.io/XRT/2022.1/html/xrt_native_apis.html
#include <experimental/xrt_xclbin.h>
#include <xrt/xrt_bo.h>
#include <xrt/xrt_device.h>
#include <xrt/xrt_kernel.h>

// System includes
//#include "cmdlineparser.h"
#include <chrono>
#include <cstdlib> // std::exit, EXIT_FAILURE
#include <cstring>
#include <filesystem> // std::filesystem::path
#include <iostream>   // std::cerr
#include <stddef.h>   // size_t
#include <stdint.h>   // uint8_t
#include <stdio.h>    // printf
#include <iostream>
namespace benchmark {
// FPGA device index ; required to open FPGA
const uint8_t device_index{0};

// IDEA:
// identify shared point: in kernel::run
// only # of params change
//
// attest(msg) : return attestation
void benchmark_kernel(kernel::Kernel target_kernel,
                      std::filesystem::path kernel_xlcbin_path,
                      size_t benchmark_execution_iterations) {

  std::cout << __PRETTY_FUNCTION__ << " [1]\n";
  // access FPGA
  auto device = xrt::device(device_index);
  std::cout << __PRETTY_FUNCTION__ << " [2]\n";
  auto uuid = device.load_xclbin(kernel_xlcbin_path);
  std::cout << __PRETTY_FUNCTION__ << " [3]\n";

  auto krnl = xrt::kernel(device, uuid, kernel::kernelToString(target_kernel),
                          xrt::kernel::cu_access_mode::exclusive);

  std::cout << __PRETTY_FUNCTION__ << " [4]\n";
  std::chrono::microseconds result;

  if (target_kernel == kernel::symmetric_attest ||
      target_kernel == kernel::asymmetric_attest) {
  std::cout << __PRETTY_FUNCTION__ << " [5]\n";
    attest::benchmark_attest_kernel(result, device, krnl, target_kernel,
                                    benchmark_execution_iterations);
  } else if (target_kernel == kernel::symmetric_verify ||
             target_kernel == kernel::asymmetric_verify) {
    verify::benchmark_verify_kernel(result, device, krnl, target_kernel,
                                    benchmark_execution_iterations);
  } else if (target_kernel == kernel::empty) {
    empty::benchmark_empty_kernel(result, device, krnl,
                                  benchmark_execution_iterations);
  } else if (target_kernel == kernel::rsa) {
    rsa::benchmark_rsa_kernel(result, device, krnl,
                                  benchmark_execution_iterations);
 }
  else {
    std::cerr << "Kernel not yet benchmarkable: " << target_kernel << "\n";
    std::exit(EXIT_FAILURE);
  }

  std::cout << "RESULT (avg latency): " << result.count() << " us\n";

  // get the output from the device
  // boOut.sync(XCL_BO_SYNC_BO_FROM_DEVICE);

  // TODO: Validate results
  // if (std::memcmp(bo2_map, bufReference, DATA_SIZE))
  //    throw std::runtime_error("Value read back does not match reference");
}
} // namespace benchmark
