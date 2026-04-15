#pragma once

#include <iostream>
#include <vector>
#include "renderer.h"

class fluid_container
{
private:
    // lowest visible density
    static constexpr float threshold = 1.0f / render_str_len;

public:
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

    // constructor
    fluid_container(int Height, int Width, float DT);

    // Convert 2D coordinates (x, y) to 1D array index for a grid of size (n+2) x (n+2)
    inline int IDX(int i, int j) const { return j * (width + 2) + i; } 
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

    void initialize_simulation(float max_wind_force);
    void update_bounds();

private:
    int find_y_bound(int y_start, int y_end, int step) const;
    int find_x_bound(int y_start, int y_end, int x_start, int current_best, int step) const;
};

const int LIN_SOL_MAX = 20;

void dens_step(int boundary_t, float diff, std::vector<float>& emission_arr, fluid_container& container);
void vel_step(float viscosity, fluid_container& container);