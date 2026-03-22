#include <iostream>
#include <vector>
#include <chrono>
#include <thread>
#include <algorithm>
#include <csignal>
#include "terminal.h"
#include "test_functions.h"
#include "fluid_math.h"
#include "input_state.h"
#include "interactive.h"
#include "settings.h"

using namespace std;

void setup();
void set_print_string(string &print_string, const vector<float>& grid ,const int TERMINAL_LEN, const int TERMINAL_WIDTH);
string get_fps_overlay(float dt);
void handleInterrupt(int signum);
void shutdown(int signum);

int main()
{
    enableANSI();

    signal(SIGINT, handleInterrupt);

    ios_base::sync_with_stdio(false);

    sim_config config;
    vector<string> warnings;
    get_user_settings(config, warnings);

    if (!warnings.empty())
    {
        for (const string& w : warnings)
        {
            cout << "\033[93m" << w << "\033[0m\n";
        }

        cout << "Press ENTER to continue...";
        cin.get();
        cout << "\n\n";
    }

    setup();
    InputState input_state;

    const float TARGET_FPS = config.target_fps;
    const chrono::milliseconds FRAME_DURATION(1000 / (int)TARGET_FPS);

    fluid_container container(getTerminalHeight(), getTerminalWidth() / 2, 1.0f / TARGET_FPS);

    string print_string;
    print_string.resize(container.height * ((container.width * 2)+ 1) - 1, ' ');

    vector<float> emission_arr;
    emission_arr.resize((container.height + 2) * (container.width + 2));

    auto prev_frame_time = chrono::high_resolution_clock::now();

    bool running = true;
    while (running)
    {
        auto frame_start = chrono::high_resolution_clock::now();

        chrono::duration<float> elapsed_seconds = frame_start - prev_frame_time;
        container.dt = elapsed_seconds.count();
        prev_frame_time = frame_start;

        update_input(input_state);
        apply_user_input(config, container, input_state, emission_arr);

        vel_step(config.visc, container);
        dens_step(0, config.diff, emission_arr, container);

        set_print_string(print_string, container.dens, container.height, container.width);

        // using flush to ensure that everything is rendered immediatly.
        cout << "\033[H" << print_string;
        cout << "\033[H\033[92m" << get_fps_overlay(container.dt) << "\033[0m" << flush;

        auto target_time = frame_start + FRAME_DURATION;
        auto now = chrono::high_resolution_clock::now();

        while (chrono::high_resolution_clock::now() < target_time);
    }

    shutdown(0);

    return 0;
}

void setup()
{
    cout << "=== ASCII Smoke Simulation ===\n\n";
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

    // enters the alternate screen buffer
    cout << "\033[?1049h" << flush;

    initTerminalSize();
}

inline char map_to_char(float density, const string& str)
{
    int max_index = str.length() - 1;

    int index = (int)(density * max_index);
    
    // Clamp using the dynamic max_index
    index = std::clamp(index, 0, max_index);
    
    return str[index];
}

void set_print_string(string &print_string, const vector<float>& grid ,const int TERMINAL_LEN, const int TERMINAL_WIDTH)
{
    static string str = R"( .`'-_,:~=;!*+<>\/|?#@)";
    // static string str2 = R"( .~=co)x(O0Q&#%B@)";

    int grid_stride = TERMINAL_WIDTH + 2;
    int string_index = 0;

    for (int i = 0; i < TERMINAL_LEN; i++)
    {
        for (int j = 0; j < TERMINAL_WIDTH; j++)
        {
            int grid_x = j + 1;
            int grid_y = i + 1;

            int fluid_index = (grid_y * grid_stride) + grid_x;
            char c = map_to_char(grid[fluid_index], str);

            print_string[string_index++] = c;
            print_string[string_index++] = c;
        }

        // as long as we are not at the last row add a newline char
        if ((i + 1) != TERMINAL_LEN)
            print_string[string_index++] = '\n';
    }
}

string get_fps_overlay(float dt)
{
    static float fps_timer = 0.0f;
    static int frame_count = 0;
    static std::string fps_display = "FPS: --";

    fps_timer += dt;
    frame_count++;

    // Update the string only once per second so it is readable
    if (fps_timer >= 1.0f)
    {
        fps_display = "FPS: " + std::to_string(frame_count);
        
        // Reset counters (subtracting 1.0f instead of setting to 0 keeps the remainder for precise timing)
        fps_timer -= 1.0f;
        frame_count = 0;
    }

    return fps_display;
}

void shutdown(int signum = 0)
{
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

// Update your signal handler to just call the new shutdown function
void handleInterrupt(int signum) {
    shutdown(signum);
}