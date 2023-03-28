#!/usr/bin/env bash
# from https://github.com/Xilinx/Vitis-Tutorials/blob/2022.2/Getting_Started/Vitis/Part4-data_center.md

# set -ux

VITIS="/share/xilinx/Vitis/2022.2"
VITIS_HLS="/share/xilinx/Vitis_HLS/2022.2"

# configure Vitis env
source "$VITIS""/settings64.sh"
source /opt/xilinx/xrt/setup.sh

# point to U280 platform installation dir
export PLATFORM_REPO_PATHS="$VITIS""/platforms"

export PATH=$PATH:/share/xilinx/Vitis_HLS/2022.2/bin
export PATH=$PATH:/share/xilinx/Vitis/2022.2/bin
export PATH=$PATH:/share/xilinx/Vivado/2022.2/bin

# to find /share/xilinx/Vitis/2022.2/lib/lnx64.o/librdi_tcltasks.so
# and other xilinx libs
if [[ -z "$LD_LIBRARY_PATH" ]] ; then
	LD_LIBRARY_PATH="$VITIS""/lib"
else
	LD_LIBRARY_PATH="$LD_LIBRARY_PATH"":""$VITIS""/lib"
fi
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:
export LD_LIBRARY_PATH="$LD_LIBRARY_PATH"":$VITIS""/lib/lnx64.o"
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/share/xilinx/Vitis_HLS/2022.2/lib
export LD_LIBRARY_PATH="$LD_LIBRARY_PATH"":$VITIS_HLS""/lib/lnx64.o"
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/share/xilinx/Vivado/2022.2/lib
# export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/share/xilinx/Vitis/2022.2/lib/lnx64.o


## missing in DEBUG ENV declaration of Vitis scripts
# export XILINX="/share/xilinx"
# export XILINX_COMMON_TOOLS="/share/xilinx/Vitis/2022.2/common"
