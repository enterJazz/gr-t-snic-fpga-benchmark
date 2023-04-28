# Thanks to Job Vranish (https://spin.atomicobject.com/2016/08/26/makefile-c-projects/)
KERNEL_SYM_ATTEST_NAME = SYMMETRIC_ATTEST
KERNEL_SYM_VERIFY_NAME = SYMMETRIC_VERIFY

KERNEL_ASYM_ATTEST_NAME = ASYMMETRIC_ATTEST
KERNEL_ASYM_VERIFY_NAME = ASYMMETRIC_VERIFY

KERNEL_EMPTY_NAME = EMPTY

HOST_TARGET = $(BUILD_DIR)/host.exe

# base src dir
BUILD_DIR := ./build
SRC_DIR := ./src


KERNEL_SYM_ATTEST_OBJ = $(BUILD_DIR)/$(KERNEL_SYM_ATTEST_NAME).xo
KERNEL_SYM_VERIFY_OBJ = $(BUILD_DIR)/$(KERNEL_SYM_VERIFY_NAME).xo
KERNEL_EMPTY_OBJ = $(BUILD_DIR)/$(KERNEL_EMPTY_NAME).xo
KERNEL_SYM_OBJS := $(KERNEL_SYM_ATTEST_OBJ) $(KERNEL_SYM_VERIFY_OBJ) $(KERNEL_EMPTY_OBJ)

KERNEL_ASYM_ATTEST_OBJ = $(BUILD_DIR)/$(KERNEL_ASYM_ATTEST_NAME).xo
KERNEL_ASYM_VERIFY_OBJ = $(BUILD_DIR)/$(KERNEL_ASYM_VERIFY_NAME).xo
KERNEL_ASYM_EMPTY_OBJ = $(BUILD_DIR)/$(KERNEL_EMPTY_NAME).xo
KERNEL_ASYM_OBJS := $(KERNEL_ASYM_ATTEST_OBJ) $(KERNEL_ASYM_VERIFY_OBJ) $(KERNEL_ASYM_EMPTY_OBJ)

KERNEL_SYM_ATTEST_XCLBIN = $(BUILD_DIR)/$(KERNEL_SYM_ATTEST_NAME).xclbin
KERNEL_SYM_VERIFY_XCLBIN = $(BUILD_DIR)/$(KERNEL_SYM_VERIFY_NAME).xclbin

KERNEL_ASYM_ATTEST_XCLBIN = $(BUILD_DIR)/$(KERNEL_ASYM_ATTEST_NAME).xclbin
KERNEL_ASYM_VERIFY_XCLBIN = $(BUILD_DIR)/$(KERNEL_ASYM_VERIFY_NAME).xclbin

KERNEL_EMPTY_XCLBIN = $(BUILD_DIR)/$(KERNEL_EMPTY_NAME).xclbin


# TARGET DEF
TARGET = $(KERNEL_SYM_XCLBIN)
KERNEL_SYM_ATTEST_DEBUG_TARGET := $(BUILD_DIR)/$(KERNEL_SYM_ATTEST_NAME).out
KERNEL_SYM_VERIFY_DEBUG_TARGET := $(BUILD_DIR)/$(KERNEL_SYM_VERIFY_NAME).out

# host, kernel srcs
HOST_SRC_DIR := $(SRC_DIR)/host
HOST_SRC_COMMON_DIR := $(HOST_SRC_DIR)/common
HOST_SRC_BENCHMARK_DIR := $(HOST_SRC_DIR)/benchmark

KERNEL_SYM_SRC_DIR := $(SRC_DIR)/kernel-sym
KERNEL_ASYM_SRC_DIR := $(SRC_DIR)/kernel-asym

# external dependencies
KERNEL_SYM_DEPS_DIR := ./kernel-sym-deps
KERNEL_SYM_HMAC_DIR := $(KERNEL_SYM_DEPS_DIR)/hmac-sha256
KERNEL_SYM_SHA_DIR := $(KERNEL_SYM_DEPS_DIR)/sha256
KERNEL_SYM_COMMON_DIR := $(KERNEL_SYM_SRC_DIR)/common

# for asym
# for public key crypto
# https://monocypher.org
KERNEL_ASYM_DEPS_DIR := ./kernel-asym-deps
KERNEL_ASYM_MONOCYPHER_DIR := $(KERNEL_ASYM_DEPS_DIR)/monocypher-4.0.1
KERNEL_ASYM_MONOCYPHER_OPTIONAL_DIR := $(KERNEL_ASYM_DEPS_DIR)/monocypher-4.0.1/optional

# Find all the C and C++ files we want to compile
# Note the single quotes around the * expressions.
# The shell will incorrectly expand these otherwise,
# but we want to send the * directly to the find command.
HOST_SRCS := $(shell find $(HOST_SRC_DIR) -name '*.cpp' -or -name '*.c' -or -name '*.s')

KERNEL_SYM_DEPS_SRCS := $(shell find $(KERNEL_SYM_DEPS_DIR) $(KERNEL_SYM_COMMON_DIR) -name '*.cpp' -or -name '*.c')
KERNEL_SYM_ATTEST_SRCS := $(KERNEL_SYM_SRC_DIR)/attest.cpp $(KERNEL_SYM_DEPS_SRCS)
KERNEL_SYM_VERIFY_SRCS := $(KERNEL_SYM_SRC_DIR)/verify.cpp $(KERNEL_SYM_DEPS_SRCS)
KERNEL_EMPTY_SRCS := $(KERNEL_SYM_SRC_DIR)/empty.cpp
KERNEL_SYM_SRCS := $(KERNEL_SYM_ATTEST_SRCS) $(KERNEL_SYM_VERIFY_SRCS) $(KERNEL_EMPTY_SRCS)

KERNEL_ASYM_DEPS_SRCS := $(shell find $(KERNEL_ASYM_DEPS_DIR) -name '*.cpp' -or -name '*.c')
KERNEL_ASYM_COMMON_SRC := $(KERNEL_ASYM_SRC_DIR)/common.cpp
KERNEL_ASYM_ATTEST_SRCS := $(KERNEL_ASYM_SRC_DIR)/attest.cpp $(KERNEL_ASYM_DEPS_SRCS) $(KERNEL_ASYM_COMMON_SRC)
KERNEL_ASYM_VERIFY_SRCS := $(KERNEL_ASYM_SRC_DIR)/verify.cpp $(KERNEL_ASYM_DEPS_SRCS) $(KERNEL_ASYM_COMMON_SRC)
KERNEL_ASYM_SRCS := $(KERNEL_ASYM_ATTEST_SRCS) $(KERNEL_ASYM_VERIFY_SRCS)


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
KERNEL_SYM_LD_FLAGS = -I$(KERNEL_SYM_SRC_DIR) -I$(KERNEL_SYM_DEPS_DIR) -I$(KERNEL_SYM_HMAC_DIR) -I$(KERNEL_SYM_SHA_DIR) -I$(KERNEL_SYM_COMMON_DIR)
KERNEL_ASYM_LD_FLAGS = -I$(KERNEL_ASYM_SRC_DIR) -I$(KERNEL_ASYM_MONOCYPHER_DIR) -I$(KERNEL_ASYM_MONOCYPHER_OPTIONAL_DIR)


# variable args
NUM_BENCHMARK_ITERATIONS = 10000

## execution
HOST_EXEC_KERNEL_ATTEST_SYM_ARGS := -k $(KERNEL_SYM_ATTEST_NAME) -x $(KERNEL_SYM_ATTEST_XCLBIN) -n $(NUM_BENCHMARK_ITERATIONS)
HOST_EXEC_KERNEL_SYM_VERIFY_ARGS := -k $(KERNEL_SYM_VERIFY_NAME) -x $(KERNEL_SYM_VERIFY_XCLBIN) -n $(NUM_BENCHMARK_ITERATIONS)

HOST_EXEC_KERNEL_ATTEST_ASYM_ARGS := -k $(KERNEL_ASYM_ATTEST_NAME) -x $(KERNEL_ASYM_ATTEST_XCLBIN) -n $(NUM_BENCHMARK_ITERATIONS)
HOST_EXEC_KERNEL_ASYM_VERIFY_ARGS := -k $(KERNEL_ASYM_VERIFY_NAME) -x $(KERNEL_ASYM_VERIFY_XCLBIN) -n $(NUM_BENCHMARK_ITERATIONS)

HOST_EXEC_KERNEL_EMPTY_ARGS := -k $(KERNEL_EMPTY_NAME) -x $(KERNEL_EMPTY_XCLBIN) -n $(NUM_BENCHMARK_ITERATIONS)


# all the source files
# SRC = $(wildcard src/*.c)
# SRC += $(wildcard deps/*/*.c)

# OBJS = $(SRC:.c=.o)

$(TARGET): all

all: check-prerequisites init host platform_config kernel link
all-sym: check-prerequisites init host platform_config kernel-sym link-sym
all-asym: check-prerequisites init host platform_config kernel-asym link-asym
# all: $(TARGET)

check-prerequisites:
ifndef XILINX_XRT
	$(error `XILINX_XRT` not set; please execute `source setup.sh`)
endif

init:
	mkdir -p $(BUILD_DIR)/$(HOST_SRC_DIR)
	mkdir -p $(BUILD_DIR)/$(KERNEL_SYM_SRC_DIR)

host: $(HOST_SRCS)
	$(CPP) $(CPP_FLAGS) $(HOST_SRCS) -o $(HOST_TARGET) $(HOST_LD_FLAGS)

platform_config:
	$(CFGUTIL) --platform $(TARGET_PLATFORM) --od $(BUILD_DIR)

kernel: kernel-sym kernel-asym

kernel-sym: kernel-sym-attest kernel-sym-verify kernel-empty

kernel-asym: kernel-asym-attest kernel-asym-verify

kernel-sym-attest: $(KERNEL_SYM_ATTEST_SRCS)
	$(VC)  --kernel $(KERNEL_SYM_ATTEST_NAME) --compile $(KERNEL_VC_FLAGS) $(KERNEL_SYM_LD_FLAGS) $(KERNEL_SYM_ATTEST_SRCS) -o $(KERNEL_SYM_ATTEST_OBJ)

kernel-sym-verify: $(KERNEL_SYM_VERIFY_SRCS)
	$(VC)  --kernel $(KERNEL_SYM_VERIFY_NAME) --compile $(KERNEL_VC_FLAGS) $(KERNEL_SYM_LD_FLAGS) $(KERNEL_SYM_VERIFY_SRCS) -o $(KERNEL_SYM_VERIFY_OBJ)

kernel-empty: $(KERNEL_EMPTY_SRCS)
	$(VC)  --kernel $(KERNEL_EMPTY_NAME) --compile $(KERNEL_VC_FLAGS) $(KERNEL_SYM_LD_FLAGS) $(KERNEL_EMPTY_SRCS) -o $(KERNEL_EMPTY_OBJ)

kernel-asym-attest: $(KERNEL_ASYM_ATTEST_SRCS)
	$(VC)  --kernel $(KERNEL_ASYM_ATTEST_NAME) --compile $(KERNEL_VC_FLAGS) $(KERNEL_ASYM_LD_FLAGS) $(KERNEL_ASYM_ATTEST_SRCS) -o $(KERNEL_ASYM_ATTEST_OBJ)

kernel-asym-verify: $(KERNEL_ASYM_VERIFY_SRCS)
	$(VC)  --kernel $(KERNEL_ASYM_VERIFY_NAME) --compile $(KERNEL_VC_FLAGS) $(KERNEL_ASYM_LD_FLAGS) $(KERNEL_ASYM_VERIFY_SRCS) -o $(KERNEL_ASYM_VERIFY_OBJ)


link: link-sym link-asym

link-sym: link-sym-attest link-sym-verify link-empty

link-asym: link-asym-attest link-asym-verify

link-sym-attest:
	$(VC) --link $(KERNEL_VC_FLAGS) $(KERNEL_SYM_ATTEST_OBJ) -o $(KERNEL_SYM_ATTEST_XCLBIN)

link-sym-verify:
	$(VC) --link $(KERNEL_VC_FLAGS) $(KERNEL_SYM_VERIFY_OBJ) -o $(KERNEL_SYM_VERIFY_XCLBIN)

link-asym-attest:
	$(VC) --link $(KERNEL_VC_FLAGS) $(KERNEL_ASYM_ATTEST_OBJ) -o $(KERNEL_ASYM_ATTEST_XCLBIN)

link-asym-verify:
	$(VC) --link $(KERNEL_VC_FLAGS) $(KERNEL_ASYM_VERIFY_OBJ) -o $(KERNEL_ASYM_VERIFY_XCLBIN)

link-empty:
	$(VC) --link $(KERNEL_VC_FLAGS) $(KERNEL_EMPTY_OBJ) -o $(KERNEL_EMPTY_XCLBIN)

run-sym-attest-benchmark:
	test -e $(HOST_TARGET)
	# BENCHMARK ATTEST
ifeq ($(COMPILE_TARGET), hw)
	$(HOST_TARGET) $(HOST_EXEC_KERNEL_ATTEST_SYM_ARGS)
else
	XCL_EMULATION_MODE=$(COMPILE_TARGET) $(HOST_TARGET) $(HOST_EXEC_KERNEL_ATTEST_SYM_ARGS)
endif

run-sym-verify-benchmark:
	test -e $(HOST_TARGET)
	# BENCHMARK ATTEST
ifeq ($(COMPILE_TARGET), hw)
	$(HOST_TARGET) $(HOST_EXEC_KERNEL_SYM_VERIFY_ARGS)
else
	XCL_EMULATION_MODE=$(COMPILE_TARGET) $(HOST_TARGET) $(HOST_EXEC_KERNEL_SYM_VERIFY_ARGS)
endif

run-asym-attest-benchmark:
	test -e $(HOST_TARGET)
	# BENCHMARK ATTEST
ifeq ($(COMPILE_TARGET), hw)
	$(HOST_TARGET) $(HOST_EXEC_KERNEL_ATTEST_ASYM_ARGS)
else
	$(HOST_TARGET) $(HOST_EXEC_KERNEL_ATTEST_ASYM_ARGS)
endif

run-asym-verify-benchmark:
	test -e $(HOST_TARGET)
	# BENCHMARK ATTEST
ifeq ($(COMPILE_TARGET), hw)
	$(HOST_TARGET) $(HOST_EXEC_KERNEL_ASYM_VERIFY_ARGS)
else
	XCL_EMULATION_MODE=$(COMPILE_TARGET) $(HOST_TARGET) $(HOST_EXEC_KERNEL_ASYM_VERIFY_ARGS)
endif

run-empty-benchmark:
	test -e $(HOST_TARGET)
	# BENCHMARK EMPTY
ifeq ($(COMPILE_TARGET), hw)
	$(HOST_TARGET) $(HOST_EXEC_KERNEL_EMPTY_ARGS)
else
	XCL_EMULATION_MODE=$(COMPILE_TARGET) $(HOST_TARGET) $(HOST_EXEC_KERNEL_EMPTY_ARGS)
endif


debug-sym-verify:
	test -e $(HOST_TARGET)
	XCL_EMULATION_MODE=$(COMPILE_TARGET) $(GDB) -tui --args $(HOST_TARGET) $(HOST_EXEC_KERNEL_SYM_VERIFY_ARGS)

debug-empty:
	test -e $(HOST_TARGET)
	XCL_EMULATION_MODE=$(COMPILE_TARGET) $(GDB) -tui --args $(HOST_TARGET) $(HOST_EXEC_KERNEL_EMPTY_ARGS)


			   
check-kernel-sym: check-kernel-sym-attest check-kernel-sym-verify

check-kernel-sym-attest:
	$(CPP) $(CPP_FLAGS) $(KERNEL_SYM_LD_FLAGS) $(KERNEL_SYM_ATTEST_SRCS) -o $(KERNEL_SYM_ATTEST_DEBUG_TARGET)

check-kernel-sym-verify:
	$(CPP) $(CPP_FLAGS) $(KERNEL_SYM_LD_FLAGS) $(KERNEL_SYM_VERIFY_SRCS) -o $(KERNEL_SYM_VERIFY_DEBUG_TARGET)

clean:
	rm -rf $(BUILD_DIR)
	rm -r _x/
	rm ./v++*.log
	rm xcd.log
	rm -r .Xil/

