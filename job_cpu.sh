#!/bin/bash
#SBATCH --job-name=fluid_sim_bench
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=32             
#SBATCH --time=02:00:00               
#SBATCH --output=logs/%j_out.txt
#SBATCH --error=logs/%j_err.txt

export OMP_NUM_THREADS=$SLURM_CPUS_PER_TASK
export OMP_PLACES=cores
export OMP_PROC_BIND=close


echo "Starting simulation"
./lbm_sim



echo "Simulation complete, Rendering video..."
source venv/bin/activate
python3 plot.py

echo "Render finished"
