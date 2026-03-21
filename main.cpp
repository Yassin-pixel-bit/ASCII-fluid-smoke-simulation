#include <iostream>
#include <vector>
#include <chrono>
#include <thread>
#include <algorithm>
#include "terminal.h"
#include "test_functions.h"
#include "fluid_math.h"

using namespace std;

void setup();
void set_print_string(string &print_string, const vector<float>& grid ,const int TERMINAL_LEN, const int TERMINAL_WIDTH);

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

int main()
{
    ios_base::sync_with_stdio(false);

    setup();

    const float TARGET_FPS = 165.0f;
    const chrono::milliseconds FRAME_DURATION(1000 / (int)TARGET_FPS);

    fluid_container container(getTerminalHeight(), getTerminalWidth() / 2, 1.0f / TARGET_FPS);

    vector<char> grid(container.height * container.width, '@');
    string print_string;
    print_string.resize(container.height * ((container.width * 2)+ 1) - 1, ' ');

    // temp emision array
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

        static float total_sim_time = 0.0f;
        total_sim_time += container.dt;

        // 1. Set the source location to the top center
        int center_x = container.width / 2;
        int top_y = 2; // Start 2 cells below the ceiling so it doesn't clip the wall

        // 2. Inject Density (The heavy liquid/gas)
        // We inject it across a 3-cell wide block for a thick stream
        if (total_sim_time < 1.0f)
        {
            // 2. Inject Density
            emission_arr[container.IDX(center_x, top_y)] = 900.0f;
            emission_arr[container.IDX(center_x - 1, top_y)] = 900.0f;
            emission_arr[container.IDX(center_x + 1, top_y)] = 900.0f;

            container.vel_y_prev[container.IDX(center_x, top_y)] = 500.0f;
            container.vel_y_prev[container.IDX(center_x - 1, top_y)] = 500.0f;
            container.vel_y_prev[container.IDX(center_x + 1, top_y)] = 500.0f;
        }
        else
        {
            container.vel_y_prev[container.IDX(center_x, top_y)] = 100.0f;
            container.vel_y_prev[container.IDX(center_x - 1, top_y)] = 100.0f;
            container.vel_y_prev[container.IDX(center_x + 1, top_y)] = 100.0f;
        }

        

        // 4. Run the Physics Engine!
        vel_step(0.001f, container);
        dens_step(0, 0.00001f, emission_arr, container);

        set_print_string(print_string, container.dens, container.height, container.width);

        // using flush to ensure that everything is rendered immediatly.
        // 1. Move to top-left and draw the fluid grid
        cout << "\033[H" << print_string;
        
        // 2. Move to top-left AGAIN, set text to Bright Green (\033[92m), print FPS, reset color (\033[0m), and flush
        cout << "\033[H\033[92m" << get_fps_overlay(container.dt) << "\033[0m" << flush;

        auto target_time = frame_start + FRAME_DURATION;
        auto now = chrono::high_resolution_clock::now();

        // while (chrono::high_resolution_clock::now() < target_time);
    }

    return 0;
}

void setup()
{
    enableANSI();

    cout << "For the best experience, please maximize your terminal or press F11 now.\n";
    cout << "Press ENTER to start the simulation...";
    cin.get();

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