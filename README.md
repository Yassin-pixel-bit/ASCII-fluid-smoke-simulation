# ASCII Fluid Dynamics Engine
A real-time, interactive fluid simulation rendered entirely in the terminal using ASCII characters. Written from scratch in C++, this engine is a direct implementation of Jos Stam's highly influential "Real-Time Fluid Dynamics for Games" stable solver, featuring cross-platform asynchronous input and custom configuration parsing.


![ASCII Fluid Simulation Demo](https://raw.githubusercontent.com/Yassin-pixel-bit/ASCII-fluid/assets/Fluid_sim_example.gif)


## Features
* **Custom Physics Solver:** Implements a stable, grid-based Eulerian fluid solver (density diffusion, velocity advection, and mass-conserving projection).
* **Interactive Controls:** Cross-platform asynchronous input allowing users to inject fluid and apply directional wind forces in real-time without pausing the simulation thread.
* **Data-Driven Configuration:** Integrates a custom `.ini` parsing system (`mINI`), allowing users to tweak viscosity, diffusion, rendering fps, and fluid limits without recompiling.


## Building the Project


This project uses CMake for cross-platform compilation.


**Prerequisites:**
* A C++ compiler (MinGW `g++`, Clang, or MSVC)
* CMake (v3.10+)


**Build Steps:**
```bash
# Clone the repository
git clone https://github.com/Yassin-pixel-bit/ASCII-fluid.git
cd ASCII-fluid
```


```bash
# Generate the build files
cmake -B build/
```


```bash
# Compile the executable
cmake --build build/
```


The executable `main` / `main.exe` will be generated in the project root.


## Controls
For the best experience, maximize your terminal window or press `F11` before starting the simulation.


- `SPACE`: Pour fluid / smoke into the container.


- `W, A, S, D`: Apply directional wind forces.


- `Q` or `Ctrl+C`: Gracefully quit the simulation and restore terminal state.


## Customization (`settings.ini`)


Upon first run, the engine will generate a `settings.ini` file in the root directory. You can modify these values to change how the simulation behaves:


- `[Engine]`: Cap the `fps` to match your monitor.


- `[Simulation Values]`: Tweak `wind_force`, `fluid_amount`, or move the `spawn_x` and `spawn_y` origin points.


- `[fluid Settings]`: Modify the raw Navier-Stokes `viscosity` and `diffusion` rates.
