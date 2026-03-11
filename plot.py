import matplotlib
matplotlib.use('Agg') # CRITICAL for headless supercomputer nodes!
import matplotlib.pyplot as plt
import matplotlib.animation as animation
import pandas as pd
import numpy as np
import time

print("Loading massive CSV into memory (this might take a minute)...")
start_time = time.time()

# 1. Load the data
df = pd.read_csv('simulation_data.csv')

# 2. Calculate velocity magnitude
df['speed'] = np.sqrt(df['u_x']**2 + df['u_y']**2)

# 3. Sort values so we can easily reshape them into a 2D grid
print("Sorting and reshaping grid...")
df.sort_values(by=['step', 'y', 'x'], inplace=True)

steps = df['step'].unique()
width = 500
height = 500

# 4. Set up the plot canvas
fig, ax = plt.subplots(figsize=(8, 8))
ax.set_facecolor('black') # Makes the dark edges look sleek

# Initialize the first frame
# We use 'magma' or 'jet' for that classic CFD heat-map look
initial_grid = df[df['step'] == steps[0]]['speed'].values.reshape((height, width))
cax = ax.imshow(initial_grid, cmap='magma', origin='lower', vmin=0, vmax=0.15)

ax.set_title("LBM Airfoil Wake")
fig.colorbar(cax, ax=ax, label="Velocity Magnitude (Lattice Units)", fraction=0.046, pad=0.04)

# 5. The animation function
def animate(i):
    step_val = steps[i]
    # Extract the flat array for this step and reshape to 2D
    grid = df[df['step'] == step_val]['speed'].values.reshape((height, width))
    cax.set_array(grid)
    ax.set_title(f"LBM Airfoil Wake - Step {step_val}")
    
    # Print progress so you don't stare at a blank terminal
    if i % 10 == 0:
        print(f"Rendering frame {i}/{len(steps)}...")
        
    return cax,

# 6. Render the video
print(f"Starting render of {len(steps)} frames...")
ani = animation.FuncAnimation(fig, animate, frames=len(steps), blit=True)

# Save as MP4. (If your cluster doesn't have ffmpeg installed, 
# change this to 'wake_video.gif' and add writer='pillow')
ani.save('wake_video.gif', fps=15, writer='pillow')

print(f"Done! Video saved in {round(time.time() - start_time, 1)} seconds.")
