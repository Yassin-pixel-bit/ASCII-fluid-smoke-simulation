#include "fluid_math.h"
#include "terminal.h"

using namespace std;

int main()
{
    int SIZE = (getTerminalHeight() + 2) * (getTerminalWidth() + 2);

    vector<float> u(SIZE), v(SIZE), u_prev(SIZE), v_prev(SIZE);
	vector<float> dens(SIZE), dens_prev(SIZE);
	
    return 0;
}

/**
 * @brief helps in generating smoke/fluid from a source
 * @param size the actuall size of the grid
 * @param grid the actuall grid that the emission will apply it's effects on
 * @param emission_array the array that has the source of the smoke/fluid
 * @param dt delta time(the diffrence between the current frame and the previous frame)
 */
void add_source(int size, vector<float>& grid, vector<float>& emission_array, float dt)
{
    for (int i = 0; i < size; i++)
    {
        grid[i] += dt * emission_array[i];
    }
}

/**
 * @brief the rate of which how the fluid/gas spreads out
 * @param height The height of the grid
 * @param width The widthof the grid
 * @param boundary_t boundary type for set_bnd()
 * @param curr_state The current state of the fluid/gas
 * @param prev_state the state of the fluid/gas in the previous frame
 * @param diff The diffusion rate
 * @param dt delta time(the diffrence between the current frame and the previous frame)
 */
void diffuse(int height, int width, int boundary_t, vector<float>& curr_state, vector<float>& prev_state, float diff, float dt)
{
    float diffusion_factor = dt * diff * height * width;

    for (int k = 0; k < 20; k++)
    {
        for (int i = 1; i <= height; i++)
        {
            for (int j = 1; j <= width; j++)
            {
                curr_state[IDX(i, j, width)] = 
                    (prev_state[IDX(i,j,width)] + 
                        diffusion_factor * (
                            curr_state[IDX(i - 1, j, width)] + 
                            curr_state[IDX(i + 1, j, width)] + 
                            curr_state[IDX(i,j - 1,width)] + 
                            curr_state[IDX(i, j + 1, width)]
                        )
                    ) / (1 + 4 * diffusion_factor);
            }
        }
        // set_bnd();
    }
}

void advect(int height, int width, vector<float>& curr_state, const vector<float>& prev_state, const vector<float>& vel_x, const vector<float>& vel_y, float dt)
{
    int left_idx, top_idx, right_idx, bottom_idx;
    float back_x, back_y, inv_blend_x, inv_blend_y, blend_x, blend_y;

    int max_dim = max(width, height);
    float scaled_dt = dt * max_dim;

    for (int i = 1; i <= height; i++)
    {
        for (int j = 1; j <= width; j++)
        {
            back_x = j - scaled_dt * vel_x[IDX(j, i, width)];
            back_y = i - scaled_dt * vel_y[IDX(j, i, width)];

            if (back_x < 0.5) back_x = 0.5;
            if (back_x > width + 0.5) back_x = width + 0.5;
            if (back_y < 0.5) back_y = 0.5;
            if (back_y > height + 0.5) back_y = height + 0.5;

            left_idx = (int) back_x; right_idx = left_idx + 1;
            top_idx = (int) back_y; bottom_idx = top_idx + 1;

            blend_x = back_x - left_idx; inv_blend_x = 1 - blend_x;
            blend_y = back_y - top_idx; inv_blend_y = 1 - blend_y;

            curr_state[IDX(j,i,width)] = 
            inv_blend_x * (inv_blend_y * prev_state[IDX(left_idx, top_idx, width)] + 
                            blend_y * prev_state[IDX(left_idx, bottom_idx, width)]) + 
            blend_x * (inv_blend_y * prev_state[IDX(right_idx, top_idx, width)] + 
                            blend_y * prev_state[IDX(right_idx, bottom_idx, width)]);
        }
    }
    // set_bnd();
}