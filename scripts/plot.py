import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import matplotlib.animation as animation
import numpy as np
import time
import sys

print("Starting render")
start_time = time.time()

width = int(sys.argv[1])
height = int(sys.argv[2])
estimated_max_u_x = float(sys.argv[3])*1.15 

data = np.fromfile(f"./logs/{sys.argv[4]}/simulation_data.bin", dtype=np.float32)

nodes_per_frame = width * height
num_frames = len(data) // nodes_per_frame
print(f"Loaded {num_frames} frames in {round(time.time() - start_time, 3)} seconds!")

grid_data = data.reshape((num_frames, height, width))

fig, ax = plt.subplots(figsize=(8, 8))
ax.set_facecolor('black')

cax = ax.imshow(grid_data[0], origin='lower', cmap='magma', vmin=0.0, vmax=(estimated_max_u_x))
ax.set_title("LBM wind tunnel simulation")
fig.colorbar(cax, ax=ax, label="Velocity Magnitude (Lattice Units)", fraction=0.046, pad=0.04)

last_frame = grid_data[-1]
solid_mask = np.isnan(last_frame) | (last_frame == 0.0)

wing_overlay = np.zeros((height, width, 4))
wing_overlay[solid_mask] = [0.7, 0.7, 0.8, 1.0]

ax.imshow(wing_overlay, origin='lower')

def animate(i):
    cax.set_array(grid_data[i])
    ax.set_title(f"LBM Airfoil Wake - Frame {i}")
    return cax,

ani = animation.FuncAnimation(fig, animate, frames=num_frames, blit=True)
ani.save(f'logs/{sys.argv[4]}/wake_video.gif', fps=15, writer='pillow')

print(f"Done! Video saved in {round(time.time() - start_time, 1)} seconds.")