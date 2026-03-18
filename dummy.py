import numpy as np
import time
import sys
import os
import shutil

# 1. Boot up and ask for fullscreen
if os.name == 'nt':
    os.system('')

print("Please maximize your terminal or press F11 now.")
input("Press ENTER to initialize the simulation...")

# 2. Get dynamic terminal size and apply the Safety Margin
cols, lines = shutil.get_terminal_size()
WIDTH = cols - 1 
HEIGHT = lines - 1

# 3. Dynamically allocate the grids based on the new size
u = np.zeros((HEIGHT, WIDTH))
v = np.zeros((HEIGHT, WIDTH))
density = np.zeros((HEIGHT, WIDTH))

# Spawn a proportional block of fluid in the upper center
start_y = int(HEIGHT * 0.1)
end_y = int(HEIGHT * 0.3)
start_x = int(WIDTH * 0.3)
end_x = int(WIDTH * 0.7)
density[start_y:end_y, start_x:end_x] = 1.0

GRADIENT = " .:-=+*#%@@@@@"

def set_bnd(b, x):
    if b == 1:
        x[:, 0] = -x[:, 1]
        x[:, -1] = -x[:, -2]
    else:
        x[:, 0] = x[:, 1]
        x[:, -1] = x[:, -2]

    if b == 2:
        x[0, :] = -x[1, :]
        x[-1, :] = -x[-2, :]
    else:
        x[0, :] = x[1, :]
        x[-1, :] = x[-2, :]

    x[0, 0] = 0.5 * (x[1, 0] + x[0, 1])
    x[0, -1] = 0.5 * (x[1, -1] + x[0, -2])
    x[-1, 0] = 0.5 * (x[-2, 0] + x[-1, 1])
    x[-1, -1] = 0.5 * (x[-2, -1] + x[-1, -2])
    return x

def advect(q, u_vel, v_vel):
    y, x = np.mgrid[0:HEIGHT, 0:WIDTH]
    bx = np.clip(x - u_vel, 0.5, WIDTH - 1.5)
    by = np.clip(y - v_vel, 0.5, HEIGHT - 1.5)
    
    x0 = bx.astype(int)
    y0 = by.astype(int)
    x1 = x0 + 1
    y1 = y0 + 1
    
    s1 = bx - x0
    s0 = 1.0 - s1
    t1 = by - y0
    t0 = 1.0 - t1
    
    return (s0 * (t0 * q[y0, x0] + t1 * q[y1, x0]) +
            s1 * (t0 * q[y0, x1] + t1 * q[y1, x1]))

def project(u, v):
    div = np.zeros((HEIGHT, WIDTH))
    p = np.zeros((HEIGHT, WIDTH))
    
    div[1:-1, 1:-1] = -0.5 * (u[1:-1, 2:] - u[1:-1, :-2] + v[2:, 1:-1] - v[:-2, 1:-1])
    div = set_bnd(0, div)
    p = set_bnd(0, p)
    
    for _ in range(80):
        p[1:-1, 1:-1] = (div[1:-1, 1:-1] + p[1:-1, :-2] + p[1:-1, 2:] + 
                         p[:-2, 1:-1] + p[2:, 1:-1]) / 4.0
        p = set_bnd(0, p)
                         
    u[1:-1, 1:-1] -= 0.5 * (p[1:-1, 2:] - p[1:-1, :-2])
    v[1:-1, 1:-1] -= 0.5 * (p[2:, 1:-1] - p[:-2, 1:-1])
    
    u = set_bnd(1, u)
    v = set_bnd(2, v)
    return u, v

def render(density):
    sys.stdout.write("\033[H")
    visual_density = np.clip(density * 1.5, 0.0, 1.0)
    visual_density = np.where(visual_density < 0.1, 0.0, visual_density)
    
    flat_d = visual_density.flatten()
    char_indices = np.clip((flat_d * (len(GRADIENT) - 1)).astype(int), 0, len(GRADIENT) - 1)
    chars = np.array(list(GRADIENT))[char_indices].reshape((HEIGHT, WIDTH))
    sys.stdout.write("\n".join("".join(row) for row in chars) + "\n")
    sys.stdout.flush()

sys.stdout.write("\033[2J")

try:
    while True:
        v += 0.25 * density 
        
        # Friction to let the fluid eventually come to rest
        u *= 0.99
        v *= 0.99
        
        u = set_bnd(1, u)
        v = set_bnd(2, v)
        
        u, v = project(u, v)
        new_u = advect(u, u, v)
        new_v = advect(v, u, v)
        
        new_u = set_bnd(1, new_u)
        new_v = set_bnd(2, new_v)
        
        u, v = project(new_u, new_v)
        
        density = advect(density, u, v)
        density = set_bnd(0, density)
        
        render(density)
        
        # Reduced sleep time to try and maintain framerate on larger grids
        time.sleep(0.01)
        
except KeyboardInterrupt:
    pass