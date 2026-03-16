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


WIDTH=500
HEIGHT=500
INITIAL_U_X=0.075
TAU=0.55
MAX_STEPS=10000
SAVE_FREQ=50

echo "Starting simulation"
./lbm_sim_cpu $WIDTH $HEIGHT $INITIAL_U_X $TAU $MAX_STEPS $SAVE_FREQ



echo "Simulation complete, Rendering video..."

./venv/bin/python plot.py $WIDTH $HEIGHT $INITIAL_U_X

echo "Render finished"
