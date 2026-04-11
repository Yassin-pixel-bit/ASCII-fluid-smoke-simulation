#include "engine_timing.h"
#include <iostream>
#include <chrono>
#include <thread>

#ifdef _WIN32
    #include <windows.h>
    #include <timeapi.h>
#endif

using namespace std;
using namespace chrono;

double OS_PERIOD = 1.0; 
double OS_TOLERANCE = 1.5;

void init_engine_timing()
{
    #ifdef _WIN32
    timeBeginPeriod(1);
    #endif

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
        auto start = steady_clock::now();
        
        #ifdef _WIN32
            Sleep(1);
        #else
            this_thread::sleep_for(chrono::milliseconds(1));
        #endif
        
        auto end = steady_clock::now();
        double actual_ms = duration<double, milli>(end - start).count();

        total_time += actual_ms;
        if (actual_ms > max_time) {
            max_time = actual_ms;
        }
    }

    OS_PERIOD = total_time / iterations;

    OS_TOLERANCE = max_time - OS_PERIOD;

    if (OS_TOLERANCE < 0.5) OS_TOLERANCE = 0.5;
}

void sleep_exact(double mili_secs)
{
    auto t0 = steady_clock::now();
    auto target = t0 + nanoseconds(int64_t(mili_secs * 1e6));

    // sleep
    double ms = mili_secs - (OS_PERIOD + OS_TOLERANCE);
    int ticks = (int)(ms / OS_PERIOD);
    if (ticks > 0)
    {
        int sleep_ms = (int)(ticks * OS_PERIOD);

        #ifdef _WIN32
            // I hate windows
            Sleep(sleep_ms);
        #else
            // Standard C++ sleep for Linux/macOS
            this_thread::sleep_for(milliseconds(sleep_ms));
        #endif
    }

    while (steady_clock::now() < target)
    {
        this_thread::yield();
    }
}

void shutdown_engine_timing()
{
    // Clean up the Windows timer
    #ifdef _WIN32
        timeEndPeriod(1);
    #endif
}