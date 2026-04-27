# ASCII Fluid Dynamics Engine

> A high-performance, interactive fluid and smoke simulation running entirely in the terminal.

![C++20+](https://img.shields.io/badge/C++-20%2B-blue.svg)
![CMake](https://img.shields.io/badge/CMake-Supported-brightgreen.svg)
![Platform](https://img.shields.io/badge/Platforms-Windows%20%7C%20Linux-lightgrey.svg)

Written from scratch in C++, this is a direct implementation of Jos Stam's [Real-Time Fluid Dynamics for Games](https://graphics.cs.cmu.edu/nsp/course/15-464/Fall09/papers/StamFluidforGames.pdf) stable solver, featuring cross-platform asynchronous input, multi-threaded physics, and an optimized ANSI rendering pipeline.

![ASCII Fluid Simulation Demo](https://raw.githubusercontent.com/Yassin-pixel-bit/ascii-fluid-smoke-simulation/assets/preview.png)

<details>
  <summary><b>Table of Contents</b></summary>

- [Features](#features)
- [Python Launcher](#python-launcher)
- [Themes Showcase](#themes-showcase)
- [How it works](#how-it-works)
  - [Red-Black Gauss-Seidel](#red-black-gauss-seidel)
  - [Delta Renderer](#delta-renderer)
- [Building the Project](#building-the-project)
- [Running](#running)
- [Controls & Customization](#controls)

</details>

## Features

- **Stable Fluid Solver:** Grid-based Eulerian simulation with density diffusion, velocity advection, and mass-conserving projection.
- **Multi-threaded Physics:** The linear solver and advection steps are parallelized using persistent `jthread` workers and a custom spin barrier, chosen over `std::barrier` for lower overhead.
- **Delta Renderer:** Only redraws cells that changed since the last frame, reducing terminal writes by ~80% in typical conditions.
- **8 Built-in Themes:** Choose from pre-made 24-bit ANSI gradient themes on startup (*Cyberpunk, Uranium Core, Fire, Vaporwave*, and more).
- **Asynchronous Input:** Fluid injection and wind forces are applied in real-time without blocking the simulation thread.
- **Dynamic HUD:** A full-width status bar showing FPS, active theme, and controls. Updated once per second to avoid impacting simulation performance.
- **INI Configuration:** Powered by [`mINI`](https://github.com/metayeti/mINI), a lightweight header-only parser. Tweak viscosity, diffusion, FPS cap, and more without recompiling.
- **GUI Launcher:** A `customtkinter` configuration window for adjusting all simulation settings before launch.

## Python Launcher

The launcher is a small `customtkinter` GUI that lets you configure all simulation settings before starting the engine. It reads and writes `settings.ini` directly, so any changes take effect immediately on launch.

<img src="https://raw.githubusercontent.com/Yassin-pixel-bit/ascii-fluid-smoke-simulation/assets/launcher.png" alt="Launcher Screenshot" width="400">

On launch, it automatically detects your **physical core count** using `psutil`, explicitly ignoring hyperthreaded logical cores, and writes that value to `settings.ini` under `[Engine] > physics_threads`. This is the only way that key gets written.

> **Note:** If you run the executable directly without having used the launcher at least once, the `physics_threads` key won't exist in `settings.ini` and the simulation will default to **1 thread**. The launcher is the recommended way to start the engine.

The launcher also exposes a **Reset** button that restores all settings to their default values.

## Themes Showcase

| Arcane Forest | Cyberpunk | Pastel Sunrise |
| :---: | :---: | :---: |
| ![Arcane Forest](https://raw.githubusercontent.com/Yassin-pixel-bit/ascii-fluid-smoke-simulation/assets/arcane_forest.gif) | ![Cyberpunk](https://raw.githubusercontent.com/Yassin-pixel-bit/ascii-fluid-smoke-simulation/assets/cyperpunk.gif) | ![Pastel Sunrise](https://raw.githubusercontent.com/Yassin-pixel-bit/ascii-fluid-smoke-simulation/assets/pastel_sunrise.gif) |

## How it works

Each cell in the grid stores a density value between 0.0 and 1.0, representing how much fluid or smoke occupies it. That value is what gets mapped to a character and a color every frame.

Each frame, the simulation runs two separate passes: one for velocity, one for density. The velocity field guides the fluid and is calculated first, then used to move the density around.

**Velocity pass:** `diffuse` → `project` → `advect`

**Density pass:** `diffuse` → `advect` *(using the velocity that was calculated)*

`Diffusion` is the natural spreading of the fluid over time with no external influence. `Projection` enforces that no velocity is created or lost without reason. `Advection` moves values through the field by tracing where each cell's content came from in the previous frame rather than pushing it forward, which is what keeps the solver numerically stable at real-time timesteps. The full mathematical details are in [Stam's paper](https://graphics.cs.cmu.edu/nsp/course/15-464/Fall09/papers/StamFluidforGames.pdf).

### Red-Black Gauss-Seidel

Both `diffusion` and `projection` rely on a `linear solver`. The original paper uses [Gauss-Seidel](https://en.wikipedia.org/wiki/Gauss%E2%80%93Seidel_method), but standard Gauss-Seidel reads neighbors that are being written to simultaneously, making it unsafe to parallelize. [Red-Black Gauss-Seidel](https://faculty.washington.edu/rjl/am583/slides2011/am583lecture22nup4.pdf) solves this by splitting the grid into a checkerboard pattern. Red cells only read from black cells and vice versa, so each color can be solved fully in parallel.

### Active Bounding Box

Each frame, the simulation tracks the min/max extent of non-empty cells and adds a safety buffer equal to the maximum distance a cell can travel in one frame. Any cell outside that box is skipped entirely. This only applies to the density pass; velocity is always computed on the full grid.

### Delta Renderer

The renderer tracks the last displayed state of every cell. Each frame it only repositions the cursor and redraws cells that actually changed, skipping the rest. The terminal is the real bottleneck at real-time framerates, especially with 24-bit color escape sequences, so this matters more than it might seem.

### Lower-Level Optimizations

- **`-ffast-math`** is enabled, disabling strict [IEEE 754](https://en.wikipedia.org/wiki/IEEE_754) compliance. This flushes computationally expensive [subnormal numbers](https://en.wikipedia.org/wiki/Subnormal_number) directly to zero. Bypassing this strict float overhead yields a massive ~44% performance increase.
- **[Row-major loop order](https://en.wikipedia.org/wiki/Row-_and_column-major_order)** is used throughout to keep memory access sequential and cache-friendly.
- **The `IDX()` macro** (which maps 2D coordinates to a flat array index) is avoided in hot inner loops where possible. The multiplication it performs creates non-sequential memory access patterns that are harder for the CPU prefetcher to predict.

## Building the Project

This project uses CMake for cross-platform compilation.

**Prerequisites:**

- A C++ compiler supporting **C++20 or higher** (built and tested with **C++23**)
- [CMake](https://cmake.org/download/) v3.10+
- [Python 3](https://www.python.org/) with the packages listed in `requirements.txt`

### **Build Steps**

**Clone the repository:**

```bash
git clone https://github.com/Yassin-pixel-bit/ASCII-fluid.git
cd ASCII-fluid
```

**Install Python dependencies:**

```bash
pip install -r requirements.txt
# On some Linux systems: pip install -r requirements.txt --break-system-packages
```

**Generate the build files:**

```bash
cmake -B build/
```

**Compile the executable:**

```bash
cmake --build build/
```

The executable `ASCII_fluid` / `ASCII_fluid.exe` will be placed in the `bin/` folder and the `launcher` will be in the root directory.

## Running

### Recommended: via the Launcher

Run `launcher` / `launcher.exe`. Configure your settings and press **Launch Simulation**.

This is the recommended way to start the engine. The launcher detects your physical core count and writes it to `settings.ini` before handing off to the executable.

### Direct: run the executable

You can run `ASCII_fluid` / `ASCII_fluid.exe` directly from the `bin/` folder. The engine will use whatever is already in `settings.ini`, generating a default one if it doesn't exist yet.

> **Note:** If `physics_threads` is missing from `settings.ini` (i.e. the launcher has never been run), the simulation will default to **1 thread** regardless of your hardware.

## Controls

For the best experience, maximize your terminal window or press `F11` before starting.

> For more detail, try zooming out or reducing your terminal font size. Increase `fluid_amount` and `wind_force` in `settings.ini` to compensate for the larger grid.

| Key | Action |
| --- | --- |
| `SPACE` | Pour fluid / smoke |
| `W A S D` | Apply directional wind forces |
| `R` | Reset simulation and return to menu |
| `C` | Clear all fluid instantly |
| `Q` / `Ctrl+C` | Quit and restore terminal state |

## Customization

> **Important Note:** On first run, the engine generates a `settings.ini` file inside the `bin/` folder. Run the executable from a directory where you have write permissions.

You can modify these values to change how the simulation behaves:

| Section | What you can change |
|---|---|
| `[Engine]` | `fps` cap, `use_colors` toggle (24-bit ANSI themes, defaults off for performance) |
| `[Simulation Values]` | `wind_force`, `fluid_amount`, `spawn_x` / `spawn_y` origin |
| `[fluid Settings]` | Raw Navier-Stokes `viscosity` and `diffusion` rates |
| `[Emitters]` | Radius and distribution of wind fans and fluid spawners |
