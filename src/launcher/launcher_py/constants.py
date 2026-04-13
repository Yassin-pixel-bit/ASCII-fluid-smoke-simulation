import os
import platform
import sys

# PyInstaller Cross-Platform Pathing
if getattr(sys, 'frozen', False):
    BASE_DIR = os.path.dirname(sys.executable)
else:
    BASE_DIR = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

EXE_NAME = "ASCII_fluid.exe" if platform.system() == "Windows" else "ASCII_fluid"
ENGINE_EXE_PATH = os.path.join(BASE_DIR, "bin", EXE_NAME)

ENGINE_DIR = os.path.dirname(ENGINE_EXE_PATH)

SETTINGS_PATH = os.path.join(ENGINE_DIR, "settings.ini")

DEFAULTS = {
    "Engine": {
        "fps": 165,
        "use_colors": 0
    },
    "Simulation Values": {
        "wind_force": 700.0,
        "fluid_amount": 650.0,
        "spawn_push": 500.0,
        "spawn_x": 0.5,
        "spawn_y": 0.0
    },
    "fluid Settings": {
        "viscosity": "0.001",
        "diffusion": "0.00001"
    },
    "Emitters": {
        "top_fan_radius": 5.0,
        "bottom_fan_radius": 5.0,
        "right_fan_radius": 5.0,
        "left_fan_radius": 5.0,
        "distribute_wind_force": 1,
        "fluid_emitter_radius": 10.0,
        "distribute_fluid_density": 1
    }
}

UI_SCHEMA = {
    "Engine": {
        "fps": {"type": int, "widget": "entry"},
        "use_colors": {"type": int, "widget": "switch"}
    },
    "Simulation Values": {
        "wind_force": {"type": float, "widget": "slider", "min": 0.0, "max": 5000.0},
        "fluid_amount": {"type": float, "widget": "slider", "min": 0.0, "max": 2000.0},
        "spawn_push": {"type": float, "widget": "slider", "min": 0.0, "max": 2000.0},
        "spawn_x": {"type": float, "widget": "slider", "min": 0.0, "max": 1.0},
        "spawn_y": {"type": float, "widget": "slider", "min": 0.0, "max": 1.0}
    },
    "fluid Settings": {
        "viscosity": {"type": str, "widget": "entry"},
        "diffusion": {"type": str, "widget": "entry"}
    },
    "Emitters": {
        "top_fan_radius": {"type": float, "widget": "slider", "min": 0.0, "max": 100.0},
        "bottom_fan_radius": {"type": float, "widget": "slider", "min": 0.0, "max": 100.0},
        "right_fan_radius": {"type": float, "widget": "slider", "min": 0.0, "max": 100.0},
        "left_fan_radius": {"type": float, "widget": "slider", "min": 0.0, "max": 100.0},
        "distribute_wind_force": {"type": int, "widget": "switch"},
        "fluid_emitter_radius": {"type": float, "widget": "slider", "min": 0.0, "max": 100.0},
        "distribute_fluid_density": {"type": int, "widget": "switch"}
    }
}