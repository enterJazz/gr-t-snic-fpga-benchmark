# Thanks to Job Vranish (https://spin.atomicobject.com/2016/08/26/makefile-c-projects/)
KERNEL_NAME = attest


# base src dir
BUILD_DIR := ./build
SRC_DIR := ./src

HOST_TARGET = $(BUILD_DIR)/host.exe
KERNEL_OBJ = $(BUILD_DIR)/$(KERNEL_NAME).xo
KERNEL_XCLBIN = $(BUILD_DIR)/$(KERNEL_NAME).xclbin

# TARGET DEF
TARGET = $(KERNEL_XCLBIN)

# host, kernel srcs
HOST_SRC_DIR := $(SRC_DIR)/host
KERNEL_SRC_DIR := $(SRC_DIR)/kernel

# dependencies
KERNEL_DEPS_DIR := ./kernel-deps
KERNEL_HMAC_DIR := $(KERNEL_DEPS_DIR)/hmac-sha256
KERNEL_SHA_DIR := $(KERNEL_DEPS_DIR)/sha256

# Find all the C and C++ files we want to compile
# Note the single quotes around the * expressions.
# The shell will incorrectly expand these otherwise,
# but we want to send the * directly to the find command.
HOST_SRCS := $(shell find $(HOST_SRC_DIR) -name '*.cpp' -or -name '*.c' -or -name '*.s')
KERNEL_SRCS := $(shell find $(KERNEL_SRC_DIR) $(KERNEL_DEPS_DIR) -name '*.cpp' -or -name '*.c')



# tools
## compilers
CC = gcc
CPP = g++
VC = v++
## vits FPGA config
CFGUTIL = emconfigutil

# target platform of the FPGA
TARGET_PLATFORM = xilinx_u280_gen3x16_xdma_1_202211_1
# compile target of VC [sw_emu|hw_emu|hw]
COMPILE_TARGET = sw_emu

# CFLAGS = -Ideps -Wall
HOST_CPP_FLAGS = -g -std=c++17 -Wall -O0
HOST_LD_FLAGS = -I$(XILINX_XRT)/include/ -L$(XILINX_XRT)/lib -lxrt_coreutil -pthread

KERNEL_VC_FLAGS = --target $(COMPILE_TARGET) --platform $(TARGET_PLATFORM)
KERNEL_LD_FLAGS = --kernel $(KERNEL_NAME) -I$(KERNEL_SRC_DIR) -I$(KERNEL_DEPS_DIR) -I$(KERNEL_HMAC_DIR) -I$(KERNEL_SHA_DIR)

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
	$(CPP) $(HOST_CPP_FLAGS) $(HOST_SRCS) -o $(HOST_TARGET) $(HOST_LD_FLAGS)

.PHONY:
platform_config:
	$(CFGUTIL) --platform $(TARGET_PLATFORM) --od $(BUILD_DIR)

.PHONY:
kernel: $(KERNEL_SRCS)
	$(VC) --compile $(KERNEL_VC_FLAGS) $(KERNEL_LD_FLAGS) $(KERNEL_SRCS) -o $(KERNEL_OBJ)

.PHONY:
link: $(HOST_TARGET) $(KERNEL_OBJ)
	$(VC) --link $(KERNEL_VC_FLAGS) $(KERNEL_OBJ) -o $(KERNEL_XCLBIN)

.PHONY:
run:
	test -e $(HOST_TARGET)
	XCL_EMULATION_MODE=$(COMPILE_TARGET) $(HOST_TARGET) $(KERNEL_XCLBIN)

.PHONY:
clean:
	rm -rf $(BUILD_DIR)

