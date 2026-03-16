#!/bin/bash
#SBATCH --job-name=fluid_sim_bench
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=128            
#SBATCH --time=02:00:00               
#SBATCH --output=logs/%j_out.txt
#SBATCH --error=logs/%j_err.txt
cd $SLURM_SUBMIT_DIR # Goes back to original directory to run job - so it can see the venv

export OMP_NUM_THREADS=$SLURM_CPUS_PER_TASK
export OMP_PLACES=cores
export OMP_PROC_BIND=close


WIDTH=500
HEIGHT=500
INITIAL_U_X=0.015
TAU=0.55
MAX_STEPS=50000
SAVE_FREQ=250

echo "Starting simulation"
./lbm_sim_cpu $WIDTH $HEIGHT $INITIAL_U_X $TAU $MAX_STEPS $SAVE_FREQ



echo "Simulation complete, Rendering video..."

source venv/bin/activate 
python3 plot.py $WIDTH $HEIGHT $INITIAL_U_X

echo "Render finished"
