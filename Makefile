# Thanks to Job Vranish (https://spin.atomicobject.com/2016/08/26/makefile-c-projects/)
TARGET = app.exe


# base src dir
BUILD_DIR := ./build
SRC_DIR := ./src

# intermediate targets
HOST_TARGET = $(BUILD_DIR)/app.exe

# host, kernel srcs
HOST_SRC_DIR := $(SRC_DIR)/host
KERNEL_SRC_DIR := $(SRC_DIR)/kernel

# dependencies
KERNEL_DEPS := ./kernel-deps
XILINX_XRT := /opt/xilinx/xrt

# Find all the C and C++ files we want to compile
# Note the single quotes around the * expressions.
# The shell will incorrectly expand these otherwise,
# but we want to send the * directly to the find command.
HOST_SRCS := $(shell find $(HOST_SRC_DIR) -name '*.cpp' -or -name '*.c' -or -name '*.s')
KERNEL_SRCS := $(shell find $(KERNEL_SRC_DIR) -name '*.cpp' -or -name '*.c' -or -name '*.s')
KERNEL_SRCS += $(wildcard $(KERNEL_DEPS)/*/*.c)

# Prepends BUILD_DIR and appends .o to every src file
# As an example, ./your_dir/hello.cpp turns into ./build/./your_dir/hello.cpp.o
HOST_OBJS := $(HOST_SRCS:%=$(BUILD_DIR)/%.o)
KERNEL_OBJS := $(KERNEL_SRCS:%=$(BUILD_DIR)/%.o)


# tools
## compilers
CC = gcc
CPP = g++
VC = v++
## vits FPGA config
CFGUTIL = emconfigutil

# target platform of the FPGA
TARGET_PLATFORM = xilinx_u280_gen3x16_xdma_1_202211_1


# CFLAGS = -Ideps -Wall
HOST_CPP_FLAGS = -g -std=c++17 -Wall -O0
HOST_LD_FLAGS = -I$(XILINX_XRT)/include/ -L$(XILINX_XRT)/lib -lxrt_coreutil -pthread

# all the source files
# SRC = $(wildcard src/*.c)
# SRC += $(wildcard deps/*/*.c)

# OBJS = $(SRC:.c=.o)

.PHONY:
all: init host platform_config
# all: $(TARGET)

.PHONY:
init:
	mkdir -p $(BUILD_DIR)

.PHONY:
host: $(HOST_SRCS)
	$(CPP) $(HOST_CPP_FLAGS) $(HOST_SRCS) -o $(HOST_TARGET) $(HOST_LD_FLAGS)

.PHONY:
platform_config:
	$(CFGUTIL) --platform $(TARGET_PLATFORM) --od $(BUILD_DIR)

#.PHONY:
#$(TARGET): $(OBJS)
#	$(CC) $(CFLAGS) $(LDFLAGS) -o $(TARGET) $(OBJS)

#.PHONY:
#%.o: %.c
#	$(CC) $(DEP_FLAG) $(CFLAGS) $(LDFLAGS) -o $@ -c $<

.PHONY:
clean:
	rm -rf $(BUILD_DIR)

