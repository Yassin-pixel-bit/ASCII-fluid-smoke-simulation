#pragma once

#include <stdexcept>
#include <algorithm>
#include "ini.h"

void init_ini();

struct sim_config {
    float target_fps = 60.0f;
    float wind_force = 500.0f;
    float fluid_amount = 1000.0f;
    float spawn_push = 500.0f;
    float spawn_x = 0.5f;
    float spawn_y = 0.0f;
    float visc = 0.001f;
    float diff = 0.00001f;
};

void get_user_settings(sim_config& config, std::vector<std::string>& warnings);