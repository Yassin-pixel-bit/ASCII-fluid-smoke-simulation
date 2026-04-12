import configparser
import subprocess
import platform
import shutil
import os

from constants import DEFAULTS, SETTINGS_PATH


def load_config() -> configparser.ConfigParser:
    """Create settings.ini from defaults if missing, then load and return it."""
    if not os.path.exists(SETTINGS_PATH):
        config = configparser.ConfigParser()
        for section, settings in DEFAULTS.items():
            config[section] = {k: str(v) for k, v in settings.items()}
        with open(SETTINGS_PATH, "w") as f:
            config.write(f)

    config = configparser.ConfigParser()
    config.read(SETTINGS_PATH)
    return config


def safe_read_val(config_obj, section, key, fallback, val_type=float, min_limit=None, max_limit=None):
    """Read a value from config, cast it, clamp it, and fall back to default on error."""
    try:
        raw_str = config_obj.get(section, key, fallback=str(fallback))
        if val_type == str:
            return raw_str
        val = val_type(raw_str)
        if min_limit is not None:
            val = max(min_limit, val)
        if max_limit is not None:
            val = min(val, max_limit)
        return val
    except ValueError:
        return fallback

def spawn_terminal(executable_path, working_dir):
    """Launch the engine executable in a new terminal window, cross-platform."""
    sys_os = platform.system()
    
    if sys_os == "Windows":
        subprocess.Popen(f'start "" "{executable_path}"', shell=True, cwd=working_dir)
        
    elif sys_os == "Darwin":
        subprocess.Popen(["open", "-a", "Terminal", executable_path], cwd=working_dir)
        
    else:
        terminals = ["gnome-terminal", "konsole", "xfce4-terminal", "alacritty", "xterm"]
        for term in terminals:
            if shutil.which(term):
                subprocess.Popen([term, "-e", executable_path], cwd=working_dir)
                return
            
        os.chdir(working_dir)
        os.execv(executable_path, [executable_path])