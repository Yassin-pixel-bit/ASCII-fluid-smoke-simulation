#include <iostream>
#include <vector>
#include <chrono>
#include <thread>
#include "terminal.h"
#include "test_functions.h"

using namespace std;

void setup();
void set_print_string(string &print_string, vector<char> grid ,const int TERMINAL_LEN, const int TERMINAL_WIDTH);

int main()
{
    ios_base::sync_with_stdio(false);

    setup();

    const int TERMINAL_LEN = getTerminalHeight();
    const int TERMINAL_WIDTH = getTerminalWidth();
    const int TARGET_FPS = 60;
    const chrono::milliseconds FRAME_DURATION(1000 / TARGET_FPS);

    vector<char> grid(TERMINAL_LEN * TERMINAL_WIDTH, '@');
    string print_string;
    print_string.reserve(TERMINAL_LEN * (TERMINAL_WIDTH + 1));

    bool running = true;
    while (running)
    {
        auto frame_start = chrono::high_resolution_clock::now();

        animateGrid(grid, TERMINAL_WIDTH, TERMINAL_LEN);

        set_print_string(print_string, grid, TERMINAL_LEN, TERMINAL_WIDTH);

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

void set_print_string(string &print_string, vector<char> grid ,const int TERMINAL_LEN, const int TERMINAL_WIDTH)
{
        print_string.clear();
        for (int i = 0; i < TERMINAL_LEN; i++)
        {
            for (int j = 0; j < TERMINAL_WIDTH; j++)
            {
                print_string += grid[(i * TERMINAL_WIDTH) + j];
            }
            if ((i + 1) != TERMINAL_LEN)
                print_string += "\n";
        }
}