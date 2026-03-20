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
};