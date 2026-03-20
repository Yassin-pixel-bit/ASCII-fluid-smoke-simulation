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

void lin_solve(int boundary_t, std::vector<float>& x, const std::vector<float>& x0, float a, float c, fluid_container& container)
{
    for (int k = 0; k < 20; k++)
    {
        for (int i = 1; i <= container.height; i++)
        {
            for (int j = 1; j <= container.width; j++)
            {
                x[container.IDX(i, j)] = 
                    (x0[container.IDX(i,j)] + 
                        a * (
                            x[container.IDX(i - 1, j)] + 
                            x[container.IDX(i + 1, j)] + 
                            x[container.IDX(i, j - 1)] + 
                            x[container.IDX(i, j + 1)]
                        )
                    ) / c;
            }
        }
        // set_bnd(boundary_t, x, container);
    }
}

void diffuse(int boundary_t, vector<float>& curr_state, vector<float>& prev_state, float diff, fluid_container& container)
{
    float diffusion_factor = container.dt * diff * container.height * container.width;

    lin_solve(boundary_t, curr_state, prev_state, diffusion_factor, 1 + 4 * diffusion_factor, container);
}

void advect(vector<float>& curr_state, const vector<float>& prev_state, const vector<float>& vel_x, const vector<float>& vel_y, fluid_container& container)
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
    // set_bnd();
}

void dens_step(int boundary_t, float diff, vector<float>& emission_arr, fluid_container& container)
{
    add_source(container.dens, emission_arr, container);

    swap(container.dens_prev, container.dens);
    diffuse(boundary_t, container.dens, container.dens_prev, diff, container);

    swap(container.dens_prev, container.dens);
    advect(container.dens, container.dens_prev, container.vel_x, container.vel_y, container);

    fill(emission_arr.begin(), emission_arr.end(), 0.0f);
}