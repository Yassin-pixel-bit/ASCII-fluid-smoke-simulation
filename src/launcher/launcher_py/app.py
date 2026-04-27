import sys
import os
import customtkinter as ctk

from constants import DEFAULTS, UI_SCHEMA, SETTINGS_PATH, ENGINE_EXE_PATH, BASE_DIR
from logic import load_config, safe_read_val, spawn_terminal
from widgets import link_slider_and_entry, slider_decimals

try:
    import psutil
except ImportError:
    psutil = None

def get_physical_cores():
    if psutil:
        cores = psutil.cpu_count(logical=False)
        if cores is not None:
            return cores
    return max(1, os.cpu_count() // 2)

class AppLauncher:
    def __init__(self):
        self.config = load_config()
        self.app_vars = {}

    def reset_to_defaults(self):
        """Factory Reset: Snaps all UI elements back to the hardcoded DEFAULTS."""
        for section, keys in self.app_vars.items():
            for key, ui_elements in keys.items():
                fallback = DEFAULTS[section][key]
                main_var = ui_elements["main_var"]

                if isinstance(main_var, ctk.DoubleVar):
                    main_var.set(float(fallback))
                elif isinstance(main_var, ctk.IntVar):
                    main_var.set(int(fallback))
                else:
                    main_var.set(str(fallback))

                if "entry_var" in ui_elements:
                    entry_var = ui_elements["entry_var"]
                    decimals = ui_elements["decimals"]
                    entry_var.set(f"{float(fallback):.{decimals}f}")

    def launch_engine(self):
        """Validate + save all settings, then launch the simulation executable."""
        for section, keys in self.app_vars.items():
            if not self.config.has_section(section):
                self.config.add_section(section)

            for key, ui_elements in keys.items():
                var = ui_elements["main_var"]
                val = var.get()

                if isinstance(var, ctk.StringVar):
                    try:
                        float(val)
                    except ValueError:
                        print(f"Invalid text in {key}. Reverting to default.")
                        val = str(DEFAULTS[section][key])
                        var.set(val)

                if isinstance(var, ctk.DoubleVar):
                    self.config.set(section, key, f"{val:.5f}".rstrip("0").rstrip("."))
                else:
                    self.config.set(section, key, str(val))

        if not self.config.has_section("Engine"):
            self.config.add_section("Engine")
            
        physics_threads = get_physical_cores()
        self.config.set("Engine", "physics_threads", str(physics_threads))

        with open(SETTINGS_PATH, "w") as configfile:
            self.config.write(configfile)

        print(f"Launching Engine: {ENGINE_EXE_PATH}")
        spawn_terminal(ENGINE_EXE_PATH)
        sys.exit(0)

    def build_slider_widget(self, frame, section_name, key, meta, val):
        """Build a linked slider + text-entry widget and register it in app_vars."""
        decimals = slider_decimals(meta["max"])
        slider_var = ctk.DoubleVar(value=val)
        entry_var = ctk.StringVar(value=f"{val:.{decimals}f}")

        slider_container = ctk.CTkFrame(frame, fg_color="transparent")
        slider_container.pack(fill="x", pady=(2, 0))

        val_box = ctk.CTkEntry(
            slider_container, textvariable=entry_var, width=60,
            justify="center", corner_radius=6
        )
        val_box.pack(side="right", padx=(10, 0))

        on_slider_move, validate_entry = link_slider_and_entry(
            slider_var, entry_var, meta["min"], meta["max"], decimals
        )

        val_box.bind("<Return>", validate_entry)
        val_box.bind("<FocusOut>", validate_entry)

        slider = ctk.CTkSlider(
            slider_container, from_=meta["min"], to=meta["max"],
            variable=slider_var, command=on_slider_move
        )
        slider.pack(side="left", fill="x", expand=True)

        self.app_vars[section_name][key] = {
            "main_var": slider_var,
            "entry_var": entry_var,
            "decimals": decimals
        }

    def build_switch_widget(self, frame, section_name, key, val):
        """Build a toggle switch widget and register it in app_vars."""
        var = ctk.IntVar(value=val)
        switch = ctk.CTkSwitch(frame, text="", variable=var)
        switch.pack(anchor="w", pady=(2, 0))
        self.app_vars[section_name][key] = {"main_var": var}

    def build_entry_widget(self, frame, section_name, key, val):
        """Build a plain text-entry widget and register it in app_vars."""
        var = ctk.StringVar(value=str(val))
        entry = ctk.CTkEntry(frame, textvariable=var)
        entry.pack(fill="x", pady=(2, 0))
        self.app_vars[section_name][key] = {"main_var": var}

    def build_tab(self, tabview, section_name, settings):
        """Populate a single settings tab with all its widgets."""
        tab = tabview.add(section_name)
        self.app_vars[section_name] = {}

        scroll = ctk.CTkScrollableFrame(tab, fg_color="transparent")
        scroll.pack(fill="both", expand=True)

        for key, meta in settings.items():
            fallback = DEFAULTS[section_name][key]
            val = safe_read_val(
                self.config, section_name, key, fallback,
                meta["type"], meta.get("min"), meta.get("max")
            )

            frame = ctk.CTkFrame(scroll, fg_color="transparent")
            frame.pack(fill="x", pady=7, padx=10)

            clean_name = key.replace("_", " ").title()
            ctk.CTkLabel(frame, text=clean_name, font=("Roboto", 13)).pack(anchor="w")

            if meta["widget"] == "slider":
                self.build_slider_widget(frame, section_name, key, meta, val)
            elif meta["widget"] == "switch":
                self.build_switch_widget(frame, section_name, key, val)
            elif meta["widget"] == "entry":
                self.build_entry_widget(frame, section_name, key, val)

    def build_ui(self, app):
        """Construct all tabs and the bottom button bar."""
        ctk.CTkLabel(app, text="Engine Configuration", font=("Roboto", 20, "bold")).pack(pady=(15, 5))

        tabview = ctk.CTkTabview(app)
        tabview.pack(fill="both", expand=True, padx=20, pady=10)

        for section_name, settings in UI_SCHEMA.items():
            self.build_tab(tabview, section_name, settings)

        btn_frame = ctk.CTkFrame(app, fg_color="transparent")
        btn_frame.pack(pady=(5, 20), fill="x", padx=40)

        ctk.CTkButton(
            btn_frame, text="RESET", font=("Roboto", 14, "bold"),
            height=40, width=100, fg_color="#555555", hover_color="#333333",
            command=self.reset_to_defaults
        ).pack(side="left")

        ctk.CTkButton(
            btn_frame, text="LAUNCH SIMULATION", font=("Roboto", 14, "bold"),
            height=40, command=self.launch_engine
        ).pack(side="right", fill="x", expand=True, padx=(15, 0))

    def run(self):
        """Entry point: configure CTk, build the window, start the event loop."""
        ctk.set_appearance_mode("dark")
        ctk.set_default_color_theme("blue")

        app = ctk.CTk()
        app.title("ASCII Fluid Engine Launcher")
        app.geometry("480x600")
        app.resizable(False, False)

        self.build_ui(app)

        app.focus_set()
        app.mainloop()