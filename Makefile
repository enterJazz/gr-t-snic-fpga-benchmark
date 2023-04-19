# Thanks to Job Vranish (https://spin.atomicobject.com/2016/08/26/makefile-c-projects/)
KERNEL_ATTEST_NAME = SYMMETRIC_ATTEST
KERNEL_VERIFY_NAME = SYMMETRIC_VERIFY
HOST_TARGET = $(BUILD_DIR)/host.exe

# base src dir
BUILD_DIR := ./build
SRC_DIR := ./src


KERNEL_ATTEST_OBJ = $(BUILD_DIR)/$(KERNEL_ATTEST_NAME).xo
KERNEL_VERIFY_OBJ = $(BUILD_DIR)/$(KERNEL_VERIFY_NAME).xo
KERNEL_OBJS := $(KERNEL_ATTEST_OBJ) $(KERNEL_VERIFY_OBJ)

KERNEL_ATTEST_XCLBIN = $(BUILD_DIR)/$(KERNEL_ATTEST_NAME).xclbin
KERNEL_VERIFY_XCLBIN = $(BUILD_DIR)/$(KERNEL_VERIFY_NAME).xclbin

# TARGET DEF
TARGET = $(KERNEL_XCLBIN)
KERNEL_ATTEST_DEBUG_TARGET := $(BUILD_DIR)/$(KERNEL_ATTEST_NAME).out
KERNEL_VERIFY_DEBUG_TARGET := $(BUILD_DIR)/$(KERNEL_VERIFY_NAME).out

# host, kernel srcs
HOST_SRC_DIR := $(SRC_DIR)/host
HOST_SRC_COMMON_DIR := $(HOST_SRC_DIR)/common
HOST_SRC_BENCHMARK_DIR := $(HOST_SRC_DIR)/benchmark

KERNEL_SRC_DIR := $(SRC_DIR)/kernel

# external dependencies
KERNEL_DEPS_DIR := ./kernel-deps
KERNEL_HMAC_DIR := $(KERNEL_DEPS_DIR)/hmac-sha256
KERNEL_SHA_DIR := $(KERNEL_DEPS_DIR)/sha256
KERNEL_COMMON_DIR := $(KERNEL_SRC_DIR)/common

# Find all the C and C++ files we want to compile
# Note the single quotes around the * expressions.
# The shell will incorrectly expand these otherwise,
# but we want to send the * directly to the find command.
HOST_SRCS := $(shell find $(HOST_SRC_DIR) -name '*.cpp' -or -name '*.c' -or -name '*.s')
KERNEL_DEPS_SRCS := $(shell find $(KERNEL_DEPS_DIR) $(KERNEL_COMMON_DIR) -name '*.cpp' -or -name '*.c')
KERNEL_ATTEST_SRCS := $(KERNEL_SRC_DIR)/attest.cpp $(KERNEL_DEPS_SRCS)
KERNEL_VERIFY_SRCS := $(KERNEL_SRC_DIR)/verify.cpp $(KERNEL_DEPS_SRCS)
KERNEL_SRCS := $(KERNEL_ATTEST_SRCS) $(KERNEL_VERIFY_SRCS)


# tools
## compilers
CC      = gcc
CPP     = g++
VC      = v++
GDB     = gdb
## vits FPGA config
CFGUTIL = emconfigutil

# target platform of the FPGA
TARGET_PLATFORM = xilinx_u280_gen3x16_xdma_1_202211_1
# compile target of VC [sw_emu|hw_emu|hw]
COMPILE_TARGET = sw_emu

# CFLAGS = -Ideps -Wall
CPP_FLAGS = -g -std=c++17 -Wall -O0
HOST_LD_FLAGS = -I$(XILINX_XRT)/include/ -I$(HOST_SRC_DIR) -I$(HOST_SRC_COMMON_DIR) -I$(HOST_SRC_BENCHMARK_DIR) -L$(XILINX_XRT)/lib -lxrt_coreutil -pthread

KERNEL_VC_FLAGS = --target $(COMPILE_TARGET) --platform $(TARGET_PLATFORM)
KERNEL_LD_FLAGS = -I$(KERNEL_SRC_DIR) -I$(KERNEL_DEPS_DIR) -I$(KERNEL_HMAC_DIR) -I$(KERNEL_SHA_DIR) -I$(KERNEL_COMMON_DIR)


# variable args
NUM_BENCHMARK_ITERATIONS = 10000

## execution
HOST_EXEC_ATTEST_KERNEL_ARGS := -k $(KERNEL_ATTEST_NAME) -x $(KERNEL_ATTEST_XCLBIN) -n $(NUM_BENCHMARK_ITERATIONS)
HOST_EXEC_KERNEL_VERIFY_ARGS := -k $(KERNEL_VERIFY_NAME) -x $(KERNEL_VERIFY_XCLBIN) -n $(NUM_BENCHMARK_ITERATIONS)


# all the source files
# SRC = $(wildcard src/*.c)
# SRC += $(wildcard deps/*/*.c)

# OBJS = $(SRC:.c=.o)

.PHONY:
$(TARGET): all

.PHONY:
all: check-prerequisites init host platform_config kernel link
# all: $(TARGET)

check-prerequisites:
ifndef XILINX_XRT
	$(error `XILINX_XRT` not set; please execute `source setup.sh`)
endif

.PHONY:
init:
	mkdir -p $(BUILD_DIR)/$(HOST_SRC_DIR)
	mkdir -p $(BUILD_DIR)/$(KERNEL_SRC_DIR)

.PHONY:
host: $(HOST_SRCS)
	$(CPP) $(CPP_FLAGS) $(HOST_SRCS) -o $(HOST_TARGET) $(HOST_LD_FLAGS)

.PHONY:
platform_config:
	$(CFGUTIL) --platform $(TARGET_PLATFORM) --od $(BUILD_DIR)

.PHONY:
kernel: kernel-attest kernel-verify

.PHONY:
kernel-attest: $(KERNEL_ATTEST_SRCS)
	$(VC)  --kernel $(KERNEL_ATTEST_NAME) --compile $(KERNEL_VC_FLAGS) $(KERNEL_LD_FLAGS) $(KERNEL_ATTEST_SRCS) -o $(KERNEL_ATTEST_OBJ)

.PHONY:
kernel-verify: $(KERNEL_VERIFY_SRCS)
	$(VC)  --kernel $(KERNEL_VERIFY_NAME) --compile $(KERNEL_VC_FLAGS) $(KERNEL_LD_FLAGS) $(KERNEL_VERIFY_SRCS) -o $(KERNEL_VERIFY_OBJ)

.PHONY:
link: $(HOST_TARGET) $(KERNEL_OBJ)
	$(VC) --link $(KERNEL_VC_FLAGS) $(KERNEL_ATTEST_OBJ) -o $(KERNEL_ATTEST_XCLBIN)
	$(VC) --link $(KERNEL_VC_FLAGS) $(KERNEL_VERIFY_OBJ) -o $(KERNEL_VERIFY_XCLBIN)


.PHONY:
run-attest-benchmark:
	test -e $(HOST_TARGET)
	# RUN ATTEST
	XCL_EMULATION_MODE=$(COMPILE_TARGET) $(HOST_TARGET) $(HOST_EXEC_ATTEST_KERNEL_ARGS)

.PHONY:
run-verify-benchmark:
	test -e $(HOST_TARGET)
	# RUN ATTEST
	XCL_EMULATION_MODE=$(COMPILE_TARGET) $(HOST_TARGET) $(HOST_EXEC_KERNEL_VERIFY_ARGS)


.PHONY:
debug-attest:
	test -e $(HOST_TARGET)
	# DEBUG ATTEST
	XCL_EMULATION_MODE=$(COMPILE_TARGET) $(GDB) -tui --args $(HOST_TARGET) $(HOST_EXEC_ATTEST_KERNEL_ARGS)

.PHONY:
debug-verify:
	test -e $(HOST_TARGET)
	# DEBUG ATTEST
	XCL_EMULATION_MODE=$(COMPILE_TARGET) $(GDB) -tui --args $(HOST_TARGET) $(HOST_EXEC_KERNEL_VERIFY_ARGS)

			   
.PHONY:
check-kernel: check-kernel-attest check-kernel-verify

.PHONY:
check-kernel-attest:
	$(CPP) $(CPP_FLAGS) $(KERNEL_LD_FLAGS) $(KERNEL_ATTEST_SRCS) -o $(KERNEL_ATTEST_DEBUG_TARGET)

.PHONY:
check-kernel-verify:
	$(CPP) $(CPP_FLAGS) $(KERNEL_LD_FLAGS) $(KERNEL_VERIFY_SRCS) -o $(KERNEL_VERIFY_DEBUG_TARGET)

.PHONY:
clean:
	rm -rf $(BUILD_DIR)
	rm -r _x/
	rm ./v++*.log
	rm xcd.log

