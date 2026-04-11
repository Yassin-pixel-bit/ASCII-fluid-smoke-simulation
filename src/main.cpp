#include <iostream>
#include <vector>
#include <chrono>
#include <csignal>
#include <fmt/core.h>
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
    cout << "\033[2J\033[H";

    cout << "=== ASCII Smoke Simulation ===\n\n";

    init_engine_timing();

    cout << "Controls:\n";

#ifdef _WIN32
    cout << " - Hold [SPACE] to pour smoke.\n";
    cout << " - Hold [W,A,S,D] to apply wind.\n";
#else
    cout << " - Press [SPACE] to toggle pouring smoke on/off.\n";
    cout << " - Press [W,A,S,D] to toggle wind direction on/off.\n";
#endif

        cout << " - Press [Q] to quit the simulation.\n\n";

        cout << "NOTE: You can customize your experience by changing the settings in 'settings.ini'.\n\n";

        if (!use_colors)
        {
            cout << "\033[96mNOTE: 24-bit ANSI colors are currently disabled.\n";
            cout << "You can enable them by setting 'use_colors = 1' in 'settings.ini'.\033[0m\n\n";
        }
    
#ifdef _WIN32
    cout << "\033[93mPlease use [Q] or Ctrl+C to exit safely.\033[0m\n";
    cout << "\033[93mForce-closing via 'taskkill /F' may leave older terminals in a broken state.\033[0m\n";
    cout << "\033[93mIf your terminal breaks (stuck screen or missing cursor), close the window and open a new one.\033[0m\n\n";
#else
    cout << "\033[93mPlease use [Q], Ctrl+C, or a polite 'kill' command to exit safely.\033[0m\n";
    cout << "\033[93mForce-closing the window or using 'kill -9' (SIGKILL) will break the terminal.\033[0m\n";
    cout << "\033[93mIf your terminal breaks (invisible text or missing cursor), type 'reset' and press ENTER.\033[0m\n\n";
#endif

    cout << "For the best experience, please maximize your terminal or press F11 now.\n";

    if (use_colors)
        cout << "Press ENTER to choose a color theme...";
    else
        cout << "Press ENTER to start the simulation...";

    cin.get();

    // Force the actual terminal window background to pure black - Thanks gemini :)
    cout << "\033]11;#000000\007" << flush;

    // enters the alternate screen buffer, hides the cursor forces black background
    cout << "\033[?1049h\033[40m\033[2J\033[?25l" << flush;

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