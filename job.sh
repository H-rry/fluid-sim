#!/bin/bash
#SBATCH --job-name=fluid_sim_bench
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=48
#SBATCH --time=02:00:00
#SBATCH --output=logs/%j_out.txt
#SBATCH --error=logs/%j_err.txt
#SBATCH --partition=1CN48C6G1H_MI300A_Ubuntu22
# Run the C++ Simulation
echo "Starting Serial LBM Simulation..."
#module load rocm
#rocprof-sys-run --trace -- ./lbm_sim
export OMP_NUM_THREADS=$SLURM_CPUS_PER_TASK
export OMP_PLACES=cores
export OMP_PROC_BIND=close

./lbm_sim
# Activate Python and render the video
echo "Simulation complete! Rendering video..."
source venv/bin/activate
python3 plot.py

echo "Pipeline finished completely!"
