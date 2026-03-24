#include <iostream>
#include <vector>

#ifdef _WIN32
    #include <windows.h>
    #include <timeapi.h>
#endif

#include <chrono>
#include <thread>
#include <algorithm>
#include <csignal>
#include "terminal.h"
#include "fluid_math.h"
#include "input_state.h"
#include "interactive.h"
#include "settings.h"

using namespace std;

double OS_PERIOD = 1.0; 
double OS_TOLERANCE = 1.5;

void setup();
void set_print_string(string &print_string, const vector<float>& grid ,const int TERMINAL_LEN, const int TERMINAL_WIDTH);
string get_fps_overlay(float dt);
void shutdown(int signum);
void calibrate_os_timer();
void sleep_exact(double milliseconds);

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

    #ifdef _WIN32
    timeBeginPeriod(1);
    #endif

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

    #ifdef _WIN32
    timeEndPeriod(1);
    #endif

    shutdown(0);
    return 0;

}

void calibrate_os_timer()
{
    cout << "Calibrating OS scheduler resolution...";

    const int iterations = 20;
    double total_time = 0.0;
    double max_time = 0.0;
    
    // Warm up - necessary????
    #ifdef _WIN32
        Sleep(1);
    #else
        this_thread::sleep_for(chrono::milliseconds(1));
    #endif

    for (int i = 0; i < iterations; ++i)
    {
        auto start = chrono::steady_clock::now();
        
        #ifdef _WIN32
            Sleep(1);
        #else
            this_thread::sleep_for(chrono::milliseconds(1));
        #endif
        
        auto end = chrono::steady_clock::now();
        double actual_ms = chrono::duration<double, milli>(end - start).count();

        total_time += actual_ms;
        if (actual_ms > max_time) {
            max_time = actual_ms;
        }
    }


    OS_PERIOD = total_time / iterations;

    OS_TOLERANCE = max_time - OS_PERIOD;

    if (OS_TOLERANCE < 0.5) OS_TOLERANCE = 0.5;

    cout << "Done.\n";
    cout << "Detected PERIOD: " << OS_PERIOD << "ms | TOLERANCE: " << OS_TOLERANCE << "ms\n\n";
}

void sleep_exact(double milliseconds)
{
    auto t0 = chrono::steady_clock::now();
    auto target = t0 + chrono::nanoseconds(int64_t(milliseconds * 1e6));

    // sleep
    double ms = milliseconds - (OS_PERIOD + OS_TOLERANCE);
    int ticks = (int)(ms / OS_PERIOD);
    if (ticks > 0)
    {
        int sleep_ms = (int)(ticks * OS_PERIOD);

        #ifdef _WIN32
            // I hate windows
            Sleep(sleep_ms);
        #else
            // Standard C++ sleep for Linux/macOS
            this_thread::sleep_for(chrono::milliseconds(sleep_ms));
        #endif
    }

    while (chrono::steady_clock::now() < target)
    {
        this_thread::yield();
    }
}

void setup()
{
    cout << "=== ASCII Smoke Simulation ===\n\n";

    calibrate_os_timer();

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
    // TODO: Improve preformance
    static string str = R"( .`'-_,:~=;!*+<>\/|?#@)";

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