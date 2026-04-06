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

    setup(config.use_colors);
    InputState input_state;

    gradient_theme cyberpunk_theme = {
        {0.0f, {60, 0, 120}},          // magenta-blue
        {0.15f, {150, 0, 255}},      // purple
        {0.50f, {255, 0, 128}},     // pink
        {1.0f, {0, 255, 255}}     // Cyan
    };

    init_theme(cyberpunk_theme, render_str_len);

    const float TARGET_FPS = config.target_fps;
    const chrono::microseconds FRAME_DURATION((int)(1000000.0f / TARGET_FPS));

    fluid_container container(getTerminalHeight(), getTerminalWidth() / 2, 1.0f / TARGET_FPS);

    string print_string;
    if (config.use_colors)
        print_string.reserve(container.height * container.width * 20);
    else
        print_string.reserve(container.height * ((container.width * 2)+ 1) - 1); // the old size

    vector<float> emission_arr;
    emission_arr.resize((container.height + 2) * (container.width + 2));

    auto prev_frame_time = chrono::steady_clock::now();

    bool running = true;

    while (running)
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
        apply_user_input(config, container, input_state, emission_arr);

        if (input_state.quit) 
            break;

        vel_step(config.visc, container);
        dens_step(0, config.diff, emission_arr, container);

        set_print_string(print_string, container.dens, container.height, container.width, config.use_colors);

        fmt::print("\033[H{}", print_string);
        fmt::print("\033[H\033[92m{}\033[0m", get_fps_overlay(real_frame_time));
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

    shutdown(0);
    return 0;

}

void setup(bool use_colors)
{
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
        cout << "Press ENTER to start the simulation...";
        cin.get();

    // enters the alternate screen buffer and hides the cursor
    cout << "\033[?1049h\033[?25l" << flush;

    initTerminalSize();
}

void shutdown(int signum = 0)
{
    shutdown_engine_timing();
    restoreTerminal();
    // \033[?1049l exits the alternate screen buffer
    // \033[?25h  restores the cursor
    // \033[0m    resets all colors
    cout << "\033[?1049l\033[?25h\033[0m" << flush;
    
    if (signum != 0) {
        cout << "\nSimulation terminated cleanly by interrupt (Signal " << signum << ").\n";
        exit(signum);
    } else {
        cout << "\nSimulation finished.\n";
    }
}