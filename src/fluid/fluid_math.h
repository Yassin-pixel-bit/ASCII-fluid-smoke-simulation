#pragma once

#include <iostream>
#include <vector>
#include <math.h>


struct fluid_container 
{
    int height;
    int width;
    float dt;

    // Grids
    std::vector<float> dens;
    std::vector<float> dens_prev;
    std::vector<float> vel_x;
    std::vector<float> vel_y;
    std::vector<float> vel_x_prev;
    std::vector<float> vel_y_prev;

    // active-box vars
    int min_x, max_x, min_y, max_y;
    int safety_buffer;

    // Convert 2D coordinates (x, y) to 1D array index for a grid of size (n+2) x (n+2)
    inline int IDX(int i, int j) { return j * (width + 2) + i; }

    inline void reset_bounds() 
    {
        min_x = width; max_x = 1;
        min_y = height; max_y = 1;
    }

    inline void expand_to_include(int x, int y, int radius) 
    {
        min_x = std::max(1, std::min(min_x, x - radius));
        max_x = std::min(width, std::max(max_x, x + radius));
        min_y = std::max(1, std::min(min_y, y - radius));
        max_y = std::min(height, std::max(max_y, y + radius));
    }

    // constructor
    fluid_container(int Height, int Width, float DT) : height(Height), width(Width), dt(DT)
    {
        int size = (height + 2) * (width + 2);

        dens.resize(size);
        dens_prev.resize(size);
        vel_x.resize(size);
        vel_y.resize(size);
        vel_x_prev.resize(size);
        vel_y_prev.resize(size);
        safety_buffer = 0;
        reset_bounds();
    }

    inline void clear()
    {
        std::fill(dens.begin(), dens.end(), 0.0f);
        std::fill(dens_prev.begin(), dens_prev.end(), 0.0f);
        std::fill(vel_x.begin(), vel_x.end(), 0.0f);
        std::fill(vel_y.begin(), vel_y.end(), 0.0f);
        std::fill(vel_x_prev.begin(), vel_x_prev.end(), 0.0f);
        std::fill(vel_y_prev.begin(), vel_y_prev.end(), 0.0f);

        reset_bounds();
    }

    void initialize_simulation(float max_wind_force)
    {
        // get the delta-time of 1 frame in a 60 FPS game loop
        float max_dt = 1.0f / 60.0f;
        float scaled_dt = max_dt * std::max(width, height);

        safety_buffer = std::ceil(max_wind_force * scaled_dt) + 1;
    }
};

const int LIN_SOL_MAX = 20;

void dens_step(int boundary_t, float diff, std::vector<float>& emission_arr, fluid_container& container);
void vel_step(float viscousity, fluid_container& container);