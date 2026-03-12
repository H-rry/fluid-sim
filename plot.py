import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import matplotlib.animation as animation
import numpy as np
import time

print("Loading binary data into memory...")
start_time = time.time()

width = 500
height = 500

# 1. Load the ENTIRE binary file in one line!
# dtype=np.float32 exactly matches the 'float' we saved in C++
data = np.fromfile('simulation_data.bin', dtype=np.float32)

# 2. Figure out how many frames we have based on the file size
nodes_per_frame = width * height
num_frames = len(data) // nodes_per_frame
print(f"Loaded {num_frames} frames in {round(time.time() - start_time, 3)} seconds!")

# 3. Reshape the flat data into a 3D array: (frame, y, x)
grid_data = data.reshape((num_frames, height, width))

# --- Plotting Setup ---
fig, ax = plt.subplots(figsize=(8, 8))
ax.set_facecolor('black')

# Frame 0 is just grid_data[0]
cax = ax.imshow(grid_data[0], origin='lower', cmap='magma', vmin=0.0, vmax=0.07)
ax.set_title("LBM Airfoil Wake (15 deg AoA)")
fig.colorbar(cax, ax=ax, label="Velocity Magnitude (Lattice Units)", fraction=0.046, pad=0.04)

def animate(i):
    # Grabbing the next frame is now just slicing the array. Instant!
    cax.set_array(grid_data[i])
    ax.set_title(f"LBM Airfoil Wake - Frame {i}")
    
    if i % 10 == 0:
        print(f"Rendering frame {i}/{num_frames}...")
    return cax,

# 4. Render the GIF
print(f"Starting render of {num_frames} frames...")
ani = animation.FuncAnimation(fig, animate, frames=num_frames, blit=True)

# Save as GIF using Pillow
ani.save('wake_video.gif', fps=15, writer='pillow')

print(f"Done! Video saved in {round(time.time() - start_time, 1)} seconds.")