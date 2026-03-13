# --- Variables ---
# Swap to AMD's LLVM compiler
CXX = amdclang++ 

# The GPU Offloading Flags!
# -fopenmp-targets tells it to build for AMD GPUs
# -Xopenmp-target -march=gfx942 tells it specifically to build for the MI300A
CXXFLAGS = -O3 -g -Wall -Wextra -fopenmp -fopenmp-targets=amdgcn-amd-amdhsa -Xopenmp-target=amdgcn-amd-amdhsa -march=gfx942

# Target executable names
GUI_TARGET = lbm_gui
SIM_TARGET = lbm_sim

# ... (keep your rules the exact same below this) ...
# --- Rules ---

# 1. The default rule if you just type 'make' (builds both!)
all: $(GUI_TARGET) $(SIM_TARGET)

# 2. Build the Laptop version (requires SDL2)
$(GUI_TARGET): main_gui.cpp fluids.hpp
	$(CXX) $(CXXFLAGS) main_gui.cpp -o $(GUI_TARGET) -lSDL2

# 3. Build the Supercomputer version (NO graphics, NO SDL2)
$(SIM_TARGET): main_sim.cpp fluids.hpp
	$(CXX) $(CXXFLAGS) main_sim.cpp -o $(SIM_TARGET)

# 4. Clean up everything
clean:	
	rm -f $(GUI_TARGET) $(SIM_TARGET)