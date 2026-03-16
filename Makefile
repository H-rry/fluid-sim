# --- Variables ---
# Swap to AMD's LLVM compiler

export PATH=/shared/apps/ubuntu/opt/rocm-7.2.0/llvm/bin:$PATH

GXX = amdclang++ 

# The GPU Offloading Flags!
# -fopenmp-targets tells it to build for AMD GPUs
# -Xopenmp-target -march=gfx942 tells it specifically to build for the MI300A
GXXFLAGS = -O3 -g -Wall -Wextra -fopenmp -fopenmp-targets=amdgcn-amd-amdhsa -Xopenmp-target=amdgcn-amd-amdhsa -march=gfx942

CXX = g++
CXXFLAGS = -O3 -fopenmp -Wall -Wextra

# Target executable names
CPU_TARGET = cpu
GPU_TARGET = gpu


# If I type make it builds the CPU version
all: $(CPU_TARGET)

$(CPU_TARGET): main_sim_cpu.cpp fluids_cpu.hpp
	$(CXX) $(CXXFLAGS) main_sim.cpp -o lbm_sim_cpu

$(GPU_TARGET): main_sim_gpu.cpp fluids_gpu.hpp
	$(GXX) $(GXXFLAGS) main_sim.cpp -o lbm_sim_gpu

# 4. Clean up everything
clean:	
	rm -f lbm_sim_cpu lbm_sim_gpu