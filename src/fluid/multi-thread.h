#pragma once

#include "fluid_math.h"
#include <vector>

extern int thread_count;

// Initializes the persistent jthreads
void init_fluid_threads();

// Drop-in replacement for lin_solve that dispatches work to the threads
void threaded_lin_solve(int boundary_t, std::vector<float>& x, const std::vector<float>& x0, float a, float c, fluid_container& container, bool active_box);
void threaded_advect(int boundary_t, std::vector<float>& curr_state, const std::vector<float>& prev_state, const std::vector<float>& vel_x, const std::vector<float>& vel_y, fluid_container& container, bool active_box);
void shutdown_fluid_threads();