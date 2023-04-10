/**
* Copyright (C) 2020 Xilinx, Inc
*
* Licensed under the Apache License, Version 2.0 (the "License"). You may
* not use this file except in compliance with the License. A copy of the
* License is located at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
* WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
* License for the specific language governing permissions and limitations
* under the License.
*/

//#include "cmdlineparser.h"
#include <iostream>
#include <cstring>
#include <stdint.h> // uint8_t

// XRT includes
#include "xrt/xrt_bo.h"
#include <experimental/xrt_xclbin.h>
#include "xrt/xrt_device.h"
#include "xrt/xrt_kernel.h"

// output size of SHA256 hash ; input is message hash
#define INPUT_MSG_HASH_SIZE 32
// output size of HMAC-SHA256 ; attestation is HMAC hash
#define OUTPUT_ATTESTATION_HASH_SIZE 32

// TODO: add setup for message attestation
// attest(msg) : return attestation
int main(int argc, char** argv) {

    std::cout << "argc = " << argc << std::endl;
    for(int i=0; i < argc; i++){
	std::cout << "argv[" << i << "] = " << argv[i] << std::endl;
    }

    // Read settings
    std::string binaryFile;
    if (argc < 2) {
	binaryFile = "./attest.xclbin";
    } else {
	binaryFile = argv[1];
    }

    int device_index = 0;

    std::cout << "Open the device" << device_index << std::endl;
    auto device = xrt::device(device_index);
    std::cout << "Load the xclbin " << binaryFile << std::endl;
    auto uuid = device.load_xclbin(binaryFile);

    //auto krnl = xrt::kernel(device, uuid, "attest");
    auto krnl = xrt::kernel(device, uuid, "attest", xrt::kernel::cu_access_mode::exclusive);

    std::cout << "Allocate Buffer in Global Memory\n";
    auto boIn = xrt::bo(device, INPUT_MSG_HASH_SIZE, krnl.group_id(0)); //Match kernel arguments to RTL kernel
    auto boOut = xrt::bo(device, OUTPUT_ATTESTATION_HASH_SIZE, krnl.group_id(1));

    // Map the contents of the buffer object into host memory
    // NOTE: message contents in here
    auto bo0_map = boIn.map<uint8_t*>();
    auto bo1_map = boOut.map<uint8_t*>();
    std::fill(bo0_map, bo0_map + INPUT_MSG_HASH_SIZE, 0);
    std::fill(bo1_map, bo1_map + OUTPUT_ATTESTATION_HASH_SIZE, 0);

    // Create the test data TODO: produce actual message hash here
    // int bufReference[DATA_SIZE];
    for (int i = 0; i < INPUT_MSG_HASH_SIZE; ++i) {
        bo0_map[i] = i;
        // bufReference[i] = bo0_map[i] + bo1_map[i]; //Generate check data for validation
	// TODO: create reference hash OR reference `verify()` / switch kernel
    }

    // Synchronize buffer content with device side
    std::cout << "synchronize input buffer data to device global memory\n";
    boIn.sync(XCL_BO_SYNC_BO_TO_DEVICE);

    std::cout << "Execution of the kernel\n";
    auto run = krnl(boIn, boOut);
    run.wait();

    // Get the output;
    std::cout << "Get the output data from the device" << std::endl;
    boOut.sync(XCL_BO_SYNC_BO_FROM_DEVICE);

    // TODO: Validate results
    // if (std::memcmp(bo2_map, bufReference, DATA_SIZE))
    //    throw std::runtime_error("Value read back does not match reference");

    std::cout << bo1_map;

    std::cout << "TEST PASSED\n";
    return 0;
}
