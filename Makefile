# CPU Settings
# Defaults to standard g++, but allows the user to override it (e.g., with icpc or clang++)
CXX ?= g++
CXXFLAGS ?= -O3 -fopenmp -Wall -Wextra

# GPU Settings
# Defaults to clang++, but can be overridden (e.g., nvc++, icpx, amdclang++)
GXX ?= clang++

# GPU Offload Flags (Vendor Specific!)
# Users can override this when calling 'make gpu'
# Examples:
#   AMD MI300:   -fopenmp-targets=amdgcn-amd-amdhsa -march=gfx942
#   NVIDIA A100: -fopenmp-targets=nvptx64-nvidia-cuda -march=sm_80
#   Intel GPU:   -fiopenmp -fopenmp-targets=spir64
OFFLOAD_FLAGS ?= -fopenmp-targets=amdgcn-amd-amdhsa -march=gfx942

GXXFLAGS ?= -O3 -g -Wall -Wextra -fopenmp $(OFFLOAD_FLAGS)

# Targets 
CPU_TARGET = cpu
GPU_TARGET = gpu

all: $(CPU_TARGET)

$(CPU_TARGET): src/main_sim_cpu.cpp src/fluids_cpu.hpp
	mkdir -p bin
	$(CXX) $(CXXFLAGS) src/main_sim_cpu.cpp -o bin/lbm_sim_cpu

$(GPU_TARGET): src/main_sim_gpu.cpp src/fluids_gpu.hpp
	mkdir -p bin
	$(GXX) $(GXXFLAGS) src/main_sim_gpu.cpp -o bin/lbm_sim_gpu

clean:
	rm -rf bin/