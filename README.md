# ASCII Fluid Dynamics Engine
> A high-performance, interactive fluid and smoke simulation running entirely in the terminal.

![C++17+](https://img.shields.io/badge/C++-17%2B-blue.svg)
![CMake](https://img.shields.io/badge/CMake-Supported-brightgreen.svg)
![Platform](https://img.shields.io/badge/Platforms-Windows%20%7C%20Linux-lightgrey.svg)

Written from scratch in C++, this engine is a direct implementation of Jos Stam's highly influential "Real-Time Fluid Dynamics for Games - [the paper](https://graphics.cs.cmu.edu/nsp/course/15-464/Fall09/papers/StamFluidforGames.pdf)" stable solver, featuring cross-platform asynchronous input and custom configuration parsing. It uses an optimized ANSI rendering pipeline for coloring the fluid/smoke.

![ASCII Fluid Simulation Demo](https://raw.githubusercontent.com/Yassin-pixel-bit/ascii-fluid-smoke-simulation/assets/vaporwave.gif)

## Features

* **Custom Physics Solver:** Implements a stable, grid-based Eulerian fluid solver featuring density diffusion, velocity advection, and mass-conserving projection.

* **Dynamic Rendering Pipeline:** Implements a custom rendering pipeline that maps densities to chars and its dedicated color that is decided based on a gradient/theme.

* **Pre-made themes:** Includes a built-in theme selection menu. Choose from 8 pre-made gradient themes on startup (e.g., *Cyberpunk, Uranium Core, Fire, Vaporwave*).

* **Interactive Controls:** Cross-platform asynchronous input allowing users to inject fluid and apply directional wind forces in real-time without pausing the simulation thread.

* **Dynamic HUD:** Includes a dynamic, full-width status bar tracking real-time FPS, active themes, and controls, optimized to be updated one time per second to prioritize simulation performance.

* **Data-Driven Configuration:** Integrates a custom `.ini` parsing system (`mINI`), allowing users to tweak viscosity, diffusion, rendering fps, and fluid limits without recompiling.

## Themes Showcase

**Arcane forest:**
![Arcane Forest](https://raw.githubusercontent.com/Yassin-pixel-bit/ascii-fluid-smoke-simulation/assets/arcane_forest.gif)

**Cyberpunk:**
![Cyberpunk](https://raw.githubusercontent.com/Yassin-pixel-bit/ascii-fluid-smoke-simulation/assets/cyperpunk.gif)

**Pastel sunrise:**
![Pastel sunrise](https://raw.githubusercontent.com/Yassin-pixel-bit/ascii-fluid-smoke-simulation/assets/pastel_sunrise.gif)

## Building the Project

This project uses CMake for cross-platform compilation.

**Prerequisites:**

* A C++ compiler supporting C++17 or higher (Built and tested with C++23)
* CMake (v3.10+)

### Build Steps:

**Clone the repository:**

```bash
git clone https://github.com/Yassin-pixel-bit/ASCII-fluid.git
cd ASCII-fluid
```

**Generate the build files:**

```bash
cmake -B build/
```

**Compile the executable:**

```bash
cmake --build build/
```

The executable `ASCII-fluid` / `ASCII-fluid.exe` will be generated in the project root.

## Controls

For the best experience, maximize your terminal window or press `F11` before starting the simulation.

* `SPACE`: Pour fluid / smoke into the container.

* `W, A, S, D`: Apply directional wind forces.

* `Q` or `Ctrl+C`: Gracefully quit the simulation and restore terminal state.

* `R` : Resets the simulation and returns to the main menu.

* `C` : Instantly wipes all smoke/fluid from the container without pausing the simulation.

## Customization

> **Important Note:** Upon first run, the engine will automatically generate a `settings.ini` file in your current working directory. Ensure you place and run the executable in a folder where you have write permissions and where this configuration file won't cause clutter.

You can modify these values to change how the simulation behaves:

* `[Engine]`: Cap the `fps` to match your monitor, and toggle `use_colors = 1` to enable the 24-bit ANSI gradient themes (*Defaults to 0 for maximum performance*).

* `[Simulation Values]`: Tweak `wind_force`, `fluid_amount`, or move the `spawn_x` and `spawn_y` origin points.

* `[fluid Settings]`: Modify the raw Navier-Stokes `viscosity` and `diffusion` rates.

* `[Emitters]`: Adjust the radius and distribution behavior of the wind fans and fluid spawners.
