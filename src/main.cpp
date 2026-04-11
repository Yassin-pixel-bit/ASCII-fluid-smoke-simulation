#include <iostream>
#include <vector>
#include <chrono>
#include <csignal>
#include <fmt/core.h>
#include <fmt/color.h>
#include "engine_timing.h"
#include "terminal.h"
#include "fluid_math.h"
#include "input_state.h"
#include "interactive.h"
#include "settings.h"
#include "themes.h"
#include "renderer.h"

using namespace std;

void setup(bool use_colors);
void shutdown(int signum);
inline string set_theme();

int main()
{
    enableANSI();
    
    signal(SIGINT, shutdown);
    signal(SIGTERM, shutdown);

    ios_base::sync_with_stdio(false);

    sim_config config;
    vector<string> warnings;
    get_user_settings(config, warnings);

    if (!warnings.empty())
    {
        for (const string& w : warnings)
            cout << "\033[93m" << w << "\033[0m\n";

        cout << "\n\n";
    }

    InputState input_state;

    const float TARGET_FPS = config.target_fps;
    const chrono::microseconds FRAME_DURATION((int)(1000000.0f / TARGET_FPS));

    string print_string;

    bool app_running = true;
    string active_theme = "Default";

    while (app_running)
    {
        setup(config.use_colors);

        if (config.use_colors)
        {
            cout << "\033[2J\033[H";
            active_theme = set_theme();
        }

        fluid_container container(getTerminalHeight() - 1, getTerminalWidth() / 2, 1.0f / TARGET_FPS);

        if (config.use_colors)
            print_string.reserve(container.height * container.width * 20);
        else
            print_string.reserve(container.height * ((container.width * 2)+ 1) - 1); // size without colors in mind

        vector<float> emission_arr;
        emission_arr.resize((container.height + 2) * (container.width + 2));

        auto prev_frame_time = chrono::steady_clock::now();

        bool sim_running = true;
        while (sim_running)
        {
            auto frame_start = chrono::steady_clock::now();

            chrono::duration<float> elapsed_seconds = frame_start - prev_frame_time;
            float real_frame_time = elapsed_seconds.count();
            float current_dt = real_frame_time;

            if (current_dt > 0.016f) {
                current_dt = 0.016f; 
            }
            container.dt = current_dt;

            prev_frame_time = frame_start;

            update_input(input_state);

            if (input_state.reset)
            {
                // exit the alternate buffer, restore cursor, and clear colors
                cout << "\033[?1049l\033[?25h\033[0m" << flush;

                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');

                restoreTerminal();
                flushTerminalInput();
                
                input_state.reset_state();
                
                break;
            }

            if (input_state.quit) 
            {
                app_running = false;
                sim_running = false;
            }

            apply_user_input(config, container, input_state, emission_arr);

            vel_step(config.visc, container);
            dens_step(0, config.diff, emission_arr, container);

            set_print_string(print_string, container.dens, container.height, container.width, config.use_colors);

            fmt::print("\033[2;1H{}", print_string);

            static string current_hud;
            if (update_hud(real_frame_time, active_theme, getTerminalWidth(), current_hud)) {
                fmt::print("{}", current_hud);
            }

            std::fflush(stdout);

            auto target_time = frame_start + FRAME_DURATION;
            auto now = chrono::steady_clock::now();

            if (now < target_time)
            {
                auto remaining_time = target_time - now;
                
                double remaining_ms = chrono::duration<double, milli>(remaining_time).count();
                
                sleep_exact(remaining_ms);
            }
        }
    }

    shutdown(0);
    return 0;
}

inline string set_theme()
{
    print_theme_menu();
    
    int choice;
    while (!(cin >> choice) || choice > get_themes_max() || choice < 1)
    {
        cout << "Please, select a valid option.\n";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "Select a valid option: ";
    }

    init_selected_theme(choice - 1, render_str_len);

    return get_theme_name(choice);
}

void setup(bool use_colors)
{
    fmt::print("\033[2J\033[H");

    fmt::print("=== ASCII Smoke Simulation ===\n\n");

    init_engine_timing();

    fmt::print("Controls:\n");
#ifdef _WIN32
    fmt::print(" - Hold [SPACE] to pour smoke.\n");
    fmt::print(" - Hold [W,A,S,D] to apply wind.\n");
#else
    fmt::print(" - Press [SPACE] to toggle pouring smoke on/off.\n");
    fmt::print(" - Press [W,A,S,D] to toggle wind direction on/off.\n");
#endif

    fmt::print("NOTE: You can customize your experience by changing the settings in 'settings.ini'.\n\n");

    if (!use_colors)
    {
        fmt::print(fg(fmt::terminal_color::cyan), 
            "Info: 24-bit ANSI colors are currently disabled in 'settings.ini'.\n\n");
    }

    auto warning_style = fg(fmt::terminal_color::bright_yellow);
    
#ifdef _WIN32
    fmt::print(warning_style, "Warning: use [Q] or Ctrl+C to exit, force-closing the window may break your terminal.\n");
    fmt::print(warning_style, "If your terminal breaks (stuck screen or missing cursor), close the window and open a new one.\n\n");
#else
    fmt::print(warning_style, "Warning: Please use [Q], Ctrl+C, or a polite 'kill' command to exit safely. Force-closing (SIGKILL) may break your terminal.\n");
    fmt::print(warning_style, "If your terminal breaks (invisible text or missing cursor), type 'reset' and press ENTER.\n\n");
#endif

    fmt::print("Maximize your terminal (F11) for the best experience.\n\n");

    if (use_colors)
        fmt::print("Press ENTER to choose a color theme...");
    else
        fmt::print("Press ENTER to start the simulation...");

    // flush stdout before waiting for input, otherwise the prompt text might not render!
    std::fflush(stdout);
    cin.get();

    // Force the actual terminal window background to pure black - Thanks gemini :)
    fmt::print("\033]11;#000000\007");

    // enters the alternate screen buffer, hides the cursor
    fmt::print("\033[?1049h\033[2J\033[?25l");
    std::fflush(stdout);

    initTerminalSize();
}

void shutdown(int signum = 0)
{
    shutdown_engine_timing();
    restoreTerminal();

    // Reset the terminal window background back to the user's default theme - Thanks gemini :)
    cout << "\033]111\007" << flush;

    cout << "\033[?1049l\033[?25h\033[0m" << flush;
    
    if (signum != 0) {
        cout << "\nSimulation terminated cleanly by interrupt (Signal " << signum << ").\n";
        exit(signum);
    } else {
        cout << "\nSimulation finished.\n";
    }
}