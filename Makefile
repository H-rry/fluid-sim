# --- Variables ---
# Swap to AMD's LLVM compiler
CXX = amdclang++ 

# The GPU Offloading Flags!
# -fopenmp-targets tells it to build for AMD GPUs
# -Xopenmp-target -march=gfx942 tells it specifically to build for the MI300A
CXXFLAGS = -O3 -g -Wall -Wextra -fopenmp -fopenmp-targets=amdgcn-amd-amdhsa -Xopenmp-target=amdgcn-amd-amdhsa -march=gfx942

# Target executable names
SIM_TARGET = lbm_sim


# If I type make it builds the lbm_sim
all: $(SIM_TARGET)

$(SIM_TARGET): main_sim.cpp fluids.hpp
	$(CXX) $(CXXFLAGS) main_sim.cpp -o $(SIM_TARGET)

# 4. Clean up everything
clean:	
	rm -f $(SIM_TARGET)