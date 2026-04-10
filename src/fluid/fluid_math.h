#pragma once

#include <iostream>
#include <vector>

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

    // Convert 2D coordinates (x, y) to 1D array index for a grid of size (n+2) x (n+2)
    inline int IDX(int i, int j) { return j * (width + 2) + i; }

    fluid_container(int Height, int Width, float DT) : height(Height), width(Width), dt(DT)
    {
        int size = (height + 2) * (width + 2);

        dens.resize(size);
        dens_prev.resize(size);
        vel_x.resize(size);
        vel_y.resize(size);
        vel_x_prev.resize(size);
        vel_y_prev.resize(size);
    }

    inline void clear()
    {
        std::fill(dens.begin(), dens.end(), 0.0f);
        std::fill(dens_prev.begin(), dens_prev.end(), 0.0f);
        std::fill(vel_x.begin(), vel_x.end(), 0.0f);
        std::fill(vel_y.begin(), vel_y.end(), 0.0f);
        std::fill(vel_x_prev.begin(), vel_x_prev.end(), 0.0f);
        std::fill(vel_y_prev.begin(), vel_y_prev.end(), 0.0f);
    }
};

const int LIN_SOL_MAX = 20;

void dens_step(int boundary_t, float diff, std::vector<float>& emission_arr, fluid_container& container);
void vel_step(float viscousity, fluid_container& container);