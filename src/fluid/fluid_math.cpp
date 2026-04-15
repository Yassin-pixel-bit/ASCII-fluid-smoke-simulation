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
    int grid_stride = container.width + 2;

    // Vertical Walls (Walk down by adding grid_stride)
    for (int i = 1; i <= container.height; i++)
    {
        int left_wall = i * grid_stride;
        int right_wall = left_wall + container.width + 1;

        grid[left_wall] = b == 1 ? -grid[left_wall + 1] : grid[left_wall + 1];
        grid[right_wall] = b == 1 ? -grid[right_wall - 1] : grid[right_wall - 1];
    }

    for (int i = 1; i <= container.width; i++)
    {
        int top_wall = i;
        int bottom_wall = (container.height + 1) * grid_stride + i;

        grid[top_wall] = b == 2 ? -grid[top_wall + grid_stride] : grid[top_wall + grid_stride];
        grid[bottom_wall] = b == 2 ? -grid[bottom_wall - grid_stride] : grid[bottom_wall - grid_stride];
    }

    // Corners (Hardcoded absolute indices)
    int bottom_left_corner = (container.height + 1) * grid_stride;
    grid[0] = 0.5f * (grid[1] + grid[grid_stride]);
    grid[bottom_left_corner] = 0.5f * (grid[bottom_left_corner + 1] + grid[bottom_left_corner - grid_stride]);
    grid[container.width + 1] = 0.5f * (grid[container.width] + grid[container.width + 1 + grid_stride]);
    grid[bottom_left_corner + container.width + 1] = 0.5f * (grid[bottom_left_corner + container.width] + grid[bottom_left_corner + container.width + 1 - grid_stride]);
}

void lin_solve(int boundary_t, std::vector<float>& x, const std::vector<float>& x0, float a, float c, fluid_container& container, bool active_box)
{
    int grid_stride = container.width + 2;
    float inv_c = 1.0f / c;

    int start_x = 1, start_y = 1, end_x = container.width, end_y = container.height;
    if (active_box)
    {
        start_x = container.min_x;
        start_y = container.min_y;
        end_x = container.max_x;
        end_y = container.max_y;
    }


    for (int k = 0; k < LIN_SOL_MAX; k++)
    {
        int center = (start_y * grid_stride) + start_x;

        for (int i = start_y; i <= end_y; i++)
        {
            for (int j = start_x; j <= end_x; j++)
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

            // Jump over the unused width of the grid to get to the next row
            center += (grid_stride - (end_x - start_x + 1));
        }
        set_bnd(boundary_t, x, container);
    }
}

void diffuse(int boundary_t, vector<float>& curr_state, vector<float>& prev_state, float diff, fluid_container& container, bool active_box)
{
    float diffusion_factor = container.dt * diff * container.height * container.width;
    lin_solve(boundary_t, curr_state, prev_state, diffusion_factor, 1 + 4 * diffusion_factor, container, active_box);
}

void advect(int boundary_t, vector<float>& curr_state, const vector<float>& prev_state, const vector<float>& vel_x, const vector<float>& vel_y, fluid_container& container, bool active_box)
{
    int left_idx, top_idx, right_idx, bottom_idx;
    float back_x, back_y, inv_blend_x, inv_blend_y, blend_x, blend_y;

    int max_dim = max(container.width, container.height);
    float scaled_dt = container.dt * max_dim;

    int start_x = 1, start_y = 1, end_x = container.width, end_y = container.height;
    if (active_box)
    {
        start_x = container.min_x;
        start_y = container.min_y;
        end_x = container.max_x;
        end_y = container.max_y;
    }

    for (int i = start_y; i <= end_y; i++)
    {
        for (int j = start_x; j <= end_x; j++)
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
    int grid_stride = container.width + 2;

    float half_h = -0.5f * h;
    int center = grid_stride + 1;
    for (int i = 1; i <= container.height; i++)
    {
        for (int j = 1; j <= container.width; j++)
        {
            div[center] = half_h * (u[center + 1] - u[center - 1] + 
                                    v[center + grid_stride] - v[center - grid_stride]);
            pressure[center] = 0.0f;
            center++;
        }
        // Jump walls
        center += 2;
    }
    set_bnd(0, div, container);
    set_bnd(0, pressure, container);

    lin_solve(0, pressure, div, 1.0f, 4.0f, container, false);

    float scale = 0.5f / h;
    // Reset pointer to top-left
    center = grid_stride + 1;
    for (int i = 1; i <= container.height; i++)
    {
        for (int j = 1; j <= container.width; j++)
        {
            u[center] -= scale * (pressure[center + 1] - pressure[center - 1]);
            v[center] -= scale * (pressure[center + grid_stride] - pressure[center - grid_stride]);
            center++;
        }

        // Jump walls
        center += 2;
    }
    set_bnd(1, u, container);
    set_bnd(2, v, container);
}

void dens_step(int boundary_t, float diff, vector<float>& emission_arr, fluid_container& container)
{
    container.update_bounds();

    add_source(container.dens, emission_arr, container);

    swap(container.dens_prev, container.dens);
    diffuse(boundary_t, container.dens, container.dens_prev, diff, container, true);

    swap(container.dens_prev, container.dens);
    advect(boundary_t, container.dens, container.dens_prev, container.vel_x, container.vel_y, container, true);

    fill(emission_arr.begin(), emission_arr.end(), 0.0f);
}

void vel_step(float viscousity, fluid_container& container)
{
    add_source(container.vel_x, container.vel_x_prev, container);
    add_source(container.vel_y, container.vel_y_prev, container);

    swap(container.vel_x_prev, container.vel_x); 
    diffuse(1, container.vel_x, container.vel_x_prev, viscousity, container, false);

    swap(container.vel_y_prev, container.vel_y); 
    diffuse(2, container.vel_y, container.vel_y_prev, viscousity, container, false);

    project(container.vel_x, container.vel_y, container.vel_x_prev, container.vel_y_prev, container);
    swap(container.vel_x_prev, container.vel_x);
    swap(container.vel_y_prev, container.vel_y);

    advect(1, container.vel_x, container.vel_x_prev, container.vel_x_prev, container.vel_y_prev, container, false);
    advect(2, container.vel_y, container.vel_y_prev, container.vel_x_prev, container.vel_y_prev, container,false);
    project(container.vel_x, container.vel_y, container.vel_x_prev, container.vel_y_prev, container);

    fill(container.vel_x_prev.begin(), container.vel_x_prev.end(), 0.0f);
    fill(container.vel_y_prev.begin(), container.vel_y_prev.end(), 0.0f);
}