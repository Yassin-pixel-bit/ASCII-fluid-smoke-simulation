#pragma once

#include <iostream>
#include <vector>

struct fluid_container 
{
    int height;
    int width;
    float dt;

    // Grids
    vector<float> dens;
    vector<float> dens_prev;
    vector<float> vel_x;
    vector<float> vel_y;
    vector<float> vel_x_prev;
    vector<float> vel_y_prev;

    // Convert 2D coordinates (x, y) to 1D array index for a grid of size (n+2) x (n+2)
    inline int IDX(int i, int j) { return j * (width + 2) + i; }

    fluid_container(int Height, int Width) : height(Height), width(Width)
    {
        int size = (height + 2) * (width + 2);

        dens.resize(size);
        dens_prev.resize(size);
        vel_x.resize(size);
        vel_y.resize(size);
        vel_x_prev.resize(size);
        vel_y_prev.resize(size);
    }
};

void dens_step(int boundary_t, float diff, vector<float>& emission_arr, fluid_container& container);