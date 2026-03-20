# fluid-sim
A fast C++ fluid simulation designed specifically to learn the parallelisation process on HPCs


<img src="media/airfoil_demo.gif" alt="Airfoil Wake Simulation" width="528">


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
## Troubleshooting
___
**Issue:** `python: command not found` when running job scripts

**Cause:** HPC compute nodes typically only have `python3`, not `python`.

**Fix:** If you encounter this error, run:
```bash
rm venv/bin/python venv/bin/python3 venv/bin/python3.10
ln -s /usr/bin/python3 venv/bin/python
ln -s python venv/bin/python3
ln -s python venv/bin/python3.10
source venv/bin/activate
pip install -r requirements.txt
deactivate
```

