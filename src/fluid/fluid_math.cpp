#include "fluid_math.h"
#include "terminal.h"

using namespace std;

void add_source(vector<float>& grid, vector<float>& emission_array, fluid_container& container)
{
    int size = (container.width + 2) * (container.height + 2);
    for (int i = 0; i < size; i++)
    {
        grid[i] += container.dt * emission_array[i];
    }
}

void set_bnd(int b, vector<float>& grid, fluid_container& container)
{
    for (int i = 1; i <= container.height; i++)
    {
        grid[container.IDX(0, i)] = b == 1 ? -grid[container.IDX(1,i)] : grid[container.IDX(1,i)];
        grid[container.IDX(container.width + 1, i)] = b == 1 ? -grid[container.IDX(container.width,i)] : grid[container.IDX(container.width,i)];
    }

    for (int i = 1; i <= container.width; i++)
    {
        grid[container.IDX(i, 0)] = b == 2 ? -grid[container.IDX(i,1)] : grid[container.IDX(i,1)];
        grid[container.IDX(i, container.height + 1)] = b == 2 ? -grid[container.IDX(i, container.height)] : grid[container.IDX(i, container.height)];
    }

    grid[container.IDX(0, 0)] = 
        0.5f * (grid[container.IDX(1, 0)] + grid[container.IDX(0, 1)]);
        
    grid[container.IDX(0, container.height + 1)] = 
        0.5f * (grid[container.IDX(1, container.height + 1)] + grid[container.IDX(0, container.height)]);
        
    grid[container.IDX(container.width + 1, 0)] = 
        0.5f * (grid[container.IDX(container.width, 0)] + grid[container.IDX(container.width + 1, 1)]);
        
    grid[container.IDX(container.width + 1, container.height + 1)] = 
        0.5f * (grid[container.IDX(container.width, container.height + 1)] + grid[container.IDX(container.width + 1, container.height)]);
}

void lin_solve(int boundary_t, std::vector<float>& x, const std::vector<float>& x0, float a, float c, fluid_container& container)
{
    int grid_stride = container.width + 2;
    
    float inv_c = 1.0f / c;

    for (int k = 0; k < LIN_SOL_MAX; k++)
    {
        int center = grid_stride + 1;

        for (int i = 1; i <= container.height; i++)
        {
            for (int j = 1; j <= container.width; j++)
            {
                x[center] = 
                (x0[center] + 
                    a * (
                        x[center - 1] + 
                        x[center + 1] + 
                        x[center - grid_stride] + 
                        x[center + grid_stride]
                    )
                ) * inv_c;

                center++;
            }

            // jump over the right wall and next left wall.
            center += 2;
        }
        set_bnd(boundary_t, x, container);
    }
}

void diffuse(int boundary_t, vector<float>& curr_state, vector<float>& prev_state, float diff, fluid_container& container)
{
    float diffusion_factor = container.dt * diff * container.height * container.width;

    lin_solve(boundary_t, curr_state, prev_state, diffusion_factor, 1 + 4 * diffusion_factor, container);
}

void advect(int boundary_t, vector<float>& curr_state, const vector<float>& prev_state, const vector<float>& vel_x, const vector<float>& vel_y, fluid_container& container)
{
    int left_idx, top_idx, right_idx, bottom_idx;
    float back_x, back_y, inv_blend_x, inv_blend_y, blend_x, blend_y;

    int max_dim = max(container.width, container.height);
    float scaled_dt = container.dt * max_dim;

    for (int i = 1; i <= container.height; i++)
    {
        for (int j = 1; j <= container.width; j++)
        {
            back_x = j - scaled_dt * vel_x[container.IDX(j, i)];
            back_y = i - scaled_dt * vel_y[container.IDX(j, i)];

            if (back_x < 0.5) back_x = 0.5;
            if (back_x > container.width + 0.5) back_x = container.width + 0.5;
            if (back_y < 0.5) back_y = 0.5;
            if (back_y > container.height + 0.5) back_y = container.height + 0.5;

            left_idx = (int) back_x; right_idx = left_idx + 1;
            top_idx = (int) back_y; bottom_idx = top_idx + 1;

            blend_x = back_x - left_idx; inv_blend_x = 1 - blend_x;
            blend_y = back_y - top_idx; inv_blend_y = 1 - blend_y;

            curr_state[container.IDX(j,i)] = 
            inv_blend_x * (inv_blend_y * prev_state[container.IDX(left_idx, top_idx)] + 
                            blend_y * prev_state[container.IDX(left_idx, bottom_idx)]) + 
            blend_x * (inv_blend_y * prev_state[container.IDX(right_idx, top_idx)] + 
                            blend_y * prev_state[container.IDX(right_idx, bottom_idx)]);
        }
    }
    set_bnd(boundary_t, curr_state, container);
}

void project(vector<float>& u, vector<float>& v, vector<float>& pressure, vector<float>& div, fluid_container& container)
{
    float h = 1.0f / max(container.height, container.width);
    
    for (int i = 1; i <= container.height; i++)
    {
        for (int j = 1; j <= container.width; j++)
        {
            div[container.IDX(j, i)] = -0.5 * h * (u[container.IDX(j + 1, i)] - u[container.IDX(j - 1, i)] + 
                                                   v[container.IDX(j, i + 1)] - v[container.IDX(j, i - 1)]);
            pressure[container.IDX(j, i)] = 0.0f;
        }
    }
    set_bnd(0, div, container);
    set_bnd(0, pressure, container);

    for (int k = 0; k < LIN_SOL_MAX; k++)
    {
        for (int i = 1; i <= container.height; i++)
        {
            for (int j = 1; j <= container.width; j++)
            {
                pressure[container.IDX(j, i)] = (div[container.IDX(j, i)] + pressure[container.IDX(j - 1, i)] + pressure[container.IDX(j + 1, i)] + pressure[container.IDX(j, i - 1)] + pressure[container.IDX(j, i + 1)]) / 4;
            }
        }
        set_bnd(0, pressure, container);
    }

    float scale = 0.5f / h;
    for (int i = 1; i <= container.height; i++)
    {
        for (int j = 1; j <= container.width; j++)
        {
            u[container.IDX(j, i)] -= scale * (pressure[container.IDX(j + 1, i)] - pressure[container.IDX(j - 1, i)]);
            v[container.IDX(j, i)] -= scale * (pressure[container.IDX(j, i + 1)] - pressure[container.IDX(j, i - 1)]);
        }
    }
    set_bnd(1, u, container);
    set_bnd(2, v, container);
}

void dens_step(int boundary_t, float diff, vector<float>& emission_arr, fluid_container& container)
{
    add_source(container.dens, emission_arr, container);

    swap(container.dens_prev, container.dens);
    diffuse(boundary_t, container.dens, container.dens_prev, diff, container);

    swap(container.dens_prev, container.dens);
    advect(boundary_t, container.dens, container.dens_prev, container.vel_x, container.vel_y, container);

    fill(emission_arr.begin(), emission_arr.end(), 0.0f);
}

void vel_step(float viscousity, fluid_container& container)
{
    add_source(container.vel_x, container.vel_x_prev, container);
    add_source(container.vel_y, container.vel_y_prev, container);

    swap(container.vel_x_prev, container.vel_x); diffuse(1, container.vel_x, container.vel_x_prev, viscousity, container);
    swap(container.vel_y_prev, container.vel_y); diffuse(2, container.vel_y, container.vel_y_prev, viscousity, container);

    project(container.vel_x, container.vel_y, container.vel_x_prev, container.vel_y_prev, container);
    swap(container.vel_x_prev, container.vel_x);
    swap(container.vel_y_prev, container.vel_y);

    advect(1, container.vel_x, container.vel_x_prev, container.vel_x_prev, container.vel_y_prev, container);
    advect(2, container.vel_y, container.vel_y_prev, container.vel_x_prev, container.vel_y_prev, container);
    project(container.vel_x, container.vel_y, container.vel_x_prev, container.vel_y_prev, container);

    fill(container.vel_x_prev.begin(), container.vel_x_prev.end(), 0.0f);
    fill(container.vel_y_prev.begin(), container.vel_y_prev.end(), 0.0f);
}