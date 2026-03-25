#include <iostream>
#include <vector>
#include <chrono>
#include <thread>
#include <algorithm>
#include <csignal>
#include "engine_timing.h"
#include "terminal.h"
#include "fluid_math.h"
#include "input_state.h"
#include "interactive.h"
#include "settings.h"

using namespace std;

void setup();
void set_print_string(string &print_string, const vector<float>& grid ,const int TERMINAL_LEN, const int TERMINAL_WIDTH);
string get_fps_overlay(float dt);
void shutdown(int signum);

constexpr string_view render_str = R"( .`'-_,:~=;!*+<>\/|?#@)";
constexpr int render_str_len = render_str.size();

int main()
{
    enableANSI();
    
    signal(SIGINT, shutdown);

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

    setup();
    InputState input_state;

    const float TARGET_FPS = config.target_fps;
    const chrono::microseconds FRAME_DURATION((int)(1000000.0f / TARGET_FPS));

    fluid_container container(getTerminalHeight(), getTerminalWidth() / 2, 1.0f / TARGET_FPS);

    string print_string;
    print_string.resize(container.height * ((container.width * 2)+ 1) - 1, ' ');

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

        vel_step(config.visc, container);
        dens_step(0, config.diff, emission_arr, container);

        set_print_string(print_string, container.dens, container.height, container.width);

        // using flush to ensure that everything is rendered immediatly.
        cout << "\033[H" << print_string;
        cout << "\033[H\033[92m" << get_fps_overlay(real_frame_time) << "\033[0m" << flush;

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

void setup()
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

        cout << "For the best experience, please maximize your terminal or press F11 now.\n";
        cout << "Press ENTER to start the simulation...";
        cin.get();

    // enters the alternate screen buffer and hides the cursor
    cout << "\033[?1049h\033[?25l" << flush;

    initTerminalSize();
}

inline char map_to_char(float density)
{
    int max_index = render_str_len - 1;

    int index = (int)(density * max_index);
    
    // Clamp using the dynamic max_index
    index = clamp(index, 0, max_index);
    
    return render_str[index];
}

void set_print_string(string &print_string, const vector<float>& grid ,const int TERMINAL_LEN, const int TERMINAL_WIDTH)
{
    // TODO: Improve preformance
    int grid_stride = TERMINAL_WIDTH + 2;
    int string_index = 0;

    // Skip the top boundary wall, and the first left boundary wall
    int fluid_index = grid_stride + 1;

    for (int i = 0; i < TERMINAL_LEN; i++)
    {
        for (int j = 0; j < TERMINAL_WIDTH; j++)
        {
            char c = map_to_char(grid[fluid_index]);
            fluid_index++;

            print_string[string_index++] = c;
            print_string[string_index++] = c;
        }

        // Jump over the right wall (+1) and the next row's left wall (+1)
        fluid_index += 2;

        // as long as we are not at the last row add a newline char
        if ((i + 1) != TERMINAL_LEN)
            print_string[string_index++] = '\n';
    }
}

string get_fps_overlay(float dt)
{
    static float fps_timer = 0.0f;
    static int frame_count = 0;
    static string fps_display = "FPS: --";

    fps_timer += dt;
    frame_count++;

    // Update the string only once per second so it is readable
    if (fps_timer >= 1.0f)
    {
        fps_display = "FPS: " + to_string(frame_count);
        
        // Reset counters (subtracting 1.0f instead of setting to 0 keeps the remainder for precise timing)
        fps_timer -= 1.0f;
        frame_count = 0;
    }

    return fps_display;
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