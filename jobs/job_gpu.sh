#!/bin/bash
#SBATCH --job-name=fluid_gpu_bench
#SBATCH --partition=1CN192C4G1H_MI300A_Ubuntu22   
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=1       # We don't need 48 CPU cores anymore!
#SBATCH --gres=gpu:1            # <--- ADD THIS: Request 1 full MI300A GPU!
#SBATCH --time=00:30:00               
#SBATCH --output=logs/%j/out.txt
#SBATCH --error=logs/%j/err.txt

# 1. We must add the ROCm compiler to our path so 'make' can find amdclang++
export PATH=/shared/apps/ubuntu/opt/rocm-7.2.0/llvm/bin:$PATH

# 2. Run the simulation
./lbm_sim_gpu$SLURM_JOB_ID 

# 3. Render the video
echo "Simulation complete! Rendering video..."
./venv/bin/python3 plot.py

echo "Pipeline finished completely!"


if [ ! -s "logs/err.txt" ]; then
    rm "logs/err.txt"
fi
