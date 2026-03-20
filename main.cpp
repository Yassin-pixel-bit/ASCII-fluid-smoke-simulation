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

int main()
{
    ios_base::sync_with_stdio(false);

    setup();

    const float TARGET_FPS = 60.0f;
    const chrono::milliseconds FRAME_DURATION(1000 / (int)TARGET_FPS);

    fluid_container container(getTerminalHeight(), getTerminalWidth(), 1.0f / TARGET_FPS);

    vector<char> grid(container.height * container.width, '@');
    string print_string;
    print_string.reserve(container.height * (container.width + 1));

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

        emission_arr[container.IDX(container.width/2, container.height/2)] = 10000.0f;
        dens_step(0, 0.0001f, emission_arr, container);

        set_print_string(print_string, container.dens, container.height, container.width);

        // using flush to ensure that everything is rendered immediatly.
        cout << "\033[H" << print_string << flush;

        auto frame_end = chrono::high_resolution_clock::now();
        auto elapsed = chrono::duration_cast<chrono::milliseconds>(frame_end - frame_start);

        if (elapsed < FRAME_DURATION)
        {
            this_thread::sleep_for(FRAME_DURATION - elapsed);
        }
    }

    // for (int i = 0; i < 1; i++)
    // {
    //     cout << print_string;
    // }

    // cout << "\033[" << 31 << "m" << "test to see if ANSI works" << "\033[" << 0 << "m";

    // cin.get();

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
    int index = (int)(density * 9.0f);
    
    index = clamp(index, 0, 9);
    
    return str[index];
}

void set_print_string(string &print_string, const vector<float>& grid ,const int TERMINAL_LEN, const int TERMINAL_WIDTH)
{
    print_string.clear();
    string str = " ,~>coCO8@";

    int grid_stride = TERMINAL_WIDTH + 2;

    for (int i = 0; i < TERMINAL_LEN; i++)
    {
        for (int j = 0; j < TERMINAL_WIDTH; j++)
        {
            int grid_x = j + 1;
            int grid_y = i + 1;

            int fluid_index = (grid_y * grid_stride) + grid_x;
            print_string += map_to_char(grid[fluid_index], str);
        }

        // as long as we are not at the kast row add a newline char
        if ((i + 1) != TERMINAL_LEN)
            print_string += "\n";
    }
}