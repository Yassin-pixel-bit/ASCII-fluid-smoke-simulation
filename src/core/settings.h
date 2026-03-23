#pragma once

#include <stdexcept>
#include <algorithm>
#include "ini.h"

void init_ini();

struct sim_config {
    float target_fps = 60.0f;
    float wind_force = 700.0f;
    float fluid_amount = 650.0f;
    float spawn_push = 500.0f;
    float spawn_x = 0.5f;
    float spawn_y = 0.0f;
    float visc = 0.001f;
    float diff = 0.00001f;
    float top_fan_r = 5.0f;
    float bottom_fan_r = 5.0f;
    float right_fan_r = 5.0f;
    float left_fan_r = 5.0f;
    bool dist_wind_f = true;
    float fluid_emitter_r = 10.0f;
    bool dist_fluid = true;
};

void get_user_settings(sim_config& config, std::vector<std::string>& warnings);