# fluid-sim
A fast C++ fluid simulation designed specifically to learn the parallelisation process on HPCs

## Instructions and initial set-up
___
```Bash
# Create, activate and install requirements to a virtual environment

python -m venv venv
source venv/bin/activate
pip install -r requirements.txt
deactivate

# Then you need to build the C++ file, for CPU or GPU.
make cpu/gpu

# Use the job_cpu/gpu to run the code
sbatch job_cpu/gpu

# You can then sftp/scp the GIF onto your sytem to view it
```
