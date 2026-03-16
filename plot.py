import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import matplotlib.animation as animation
import numpy as np
import time

print("Starting render")
start_time = time.time()

width = 500
height = 500        
initial_u_x = 0.075 # Left velocity in the wind tunnel


data = np.fromfile('simulation_data.bin', dtype=np.float32)

nodes_per_frame = width * height
num_frames = len(data) // nodes_per_frame
print(f"Loaded {num_frames} frames in {round(time.time() - start_time, 3)} seconds!")

grid_data = data.reshape((num_frames, height, width))

fig, ax = plt.subplots(figsize=(8, 8))
ax.set_facecolor('black')

cax = ax.imshow(grid_data[0], origin='lower', cmap='magma', vmin=0.0, vmax=(initial_u_x + 0.05))
ax.set_title("LBM wind tunnel simulation")
fig.colorbar(cax, ax=ax, label="Velocity Magnitude (Lattice Units)", fraction=0.046, pad=0.04)

def animate(i):
    cax.set_array(grid_data[i])
    ax.set_title(f"LBM Airfoil Wake - Frame {i}")
    return cax,



ani = animation.FuncAnimation(fig, animate, frames=num_frames, blit=True)
ani.save('wake_video.gif', fps=15, writer='pillow')

print(f"Done! Video saved in {round(time.time() - start_time, 1)} seconds.")