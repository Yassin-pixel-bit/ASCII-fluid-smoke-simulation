#include "interactive.h"

using namespace std;

static int cached_cells = -1;

inline int get_radius(float prct, int side)
{
    float radius_pct = prct / 100.0f;
    int max_radius = side / 2;
    return (int)(max_radius * radius_pct);
}

inline float get_dist_wind_force(bool dist, float base_force, int radius)
{
    int total_cells = (radius * 2) + 1; // +1 includes the center cell
    
    if (dist && total_cells > 0) {
        return base_force / total_cells;
    }
    return base_force;
}

void add_wind(const sim_config& config, fluid_container& container, const InputState& input_state)
{
        int center_x = container.width / 2;
        int center_y = container.height / 2;
        int edge_offset = 2;

        if (input_state.wind_w) 
        {
            int radius = get_radius(config.bottom_fan_r, container.width);
            float force = get_dist_wind_force(config.dist_wind_f, config.wind_force, radius);

            for (int i = -radius; i <= radius; i++)
            {
                int target_x = center_x + i;
                if (target_x > 0 && target_x < container.width - 1) {
                    container.vel_y_prev[container.IDX(target_x, container.height - edge_offset)] += -force;
                }
            }
        }
        if (input_state.wind_s) 
        {
            int radius = get_radius(config.top_fan_r, container.width);
            float force = get_dist_wind_force(config.dist_wind_f, config.wind_force, radius);

            for (int i = -radius; i <= radius; i++)
            {
                int target_x = center_x + i;
                if (target_x > 0 && target_x < container.width - 1) {
                    container.vel_y_prev[container.IDX(target_x, edge_offset)] += force;
                }
            }
        }
        if (input_state.wind_a) 
        {
            int radius = get_radius(config.right_fan_r, container.height);
            float force = get_dist_wind_force(config.dist_wind_f, config.wind_force, radius);

            for (int i = -radius; i <= radius; i++)
            {
                int target_y = center_y + i;
                if (target_y > 0 && target_y < container.height - 1) {
                    container.vel_x_prev[container.IDX(container.width - edge_offset, target_y)] += -force;
                }
            }
        }
        if (input_state.wind_d) 
        {
            int radius = get_radius(config.left_fan_r, container.height);
            float force = get_dist_wind_force(config.dist_wind_f, config.wind_force, radius);

            for (int i = -radius; i <= radius; i++)
            {
                int target_y = center_y + i;
                if (target_y > 0 && target_y < container.height - 1) {
                    container.vel_x_prev[container.IDX(edge_offset, target_y)] += force;
                }
            }
        }
}

inline bool in_brush_bound(int i, int j, int radius)
{
    return i * i + j * j <= radius * radius;
}

void add_sources(const sim_config& config, fluid_container& container, const InputState& input_state, vector<float>& emission_arr)
{
    int fluid_x = 2 + (int)((container.width - 3) * config.spawn_x);
    int fluid_y = 2 + (int)((container.height - 3) * config.spawn_y);

    if (input_state.pouring_smoke)
    {
        float radius_pct = config.fluid_emitter_r / 100.0f;
        int max_radius = std::min(container.width, container.height) / 2;
        int radius = (int)(max_radius * radius_pct);

        if (radius < 0) radius = 0;

        float amount = config.fluid_amount;
        float push = config.spawn_push;

        float push_dir = config.spawn_y < 0.5f ? push : -push;

        if (cached_cells == -1)
        {
            cached_cells = 0;
            int r_sq = radius * radius;

            for (int i = -radius; i <= radius; i++) 
            {
                int i_sq = i * i;
                for (int j = -radius; j <= radius; j++) 
                {
                    if (i_sq + (j * j) <= r_sq) 
                    {
                        cached_cells++;
                    }
                }
            }
        }

        if (config.dist_fluid && cached_cells > 0)
        {
            amount /= cached_cells;
            push_dir /= cached_cells;
        }

        int max_x = container.width - 1;
        int max_y = container.height - 1;

        int grid_stride = container.width + 2;
        int spawn_idx = (fluid_y * grid_stride) + fluid_x;
        int r_sq = radius * radius;


        for (int i = -radius; i <= radius; i++) 
        {
            int target_y = fluid_y + i;

            // Skip this entire row if it's out of vertical bounds
            if (target_y <= 0 || target_y >= max_y) continue;

            int i_sq = i * i;
            int row_offset = i * grid_stride;

            for (int j = -radius; j <= radius; j++) 
            {
                int target_x = fluid_x + j;

                // Skip this cell if it's out of horizontal bounds
                if (target_x <= 0 || target_x >= max_x) continue;

                // Check if the current offset is inside the circle
                if (i_sq + (j * j) <= r_sq) 
                {
                    int final_idx = spawn_idx + row_offset + j;
                    
                    emission_arr[final_idx] += amount;
                    container.vel_y_prev[final_idx] += push_dir;
                }
            }
        }
    }
}

void update_input(InputState& input_state)
{
    // Read all input
    updateInput();

    updateActionState(input_state.pouring_smoke , ' ');
    updateActionState(input_state.wind_w, 'w');
    updateActionState(input_state.wind_s, 's');
    updateActionState(input_state.wind_a, 'a');
    updateActionState(input_state.wind_d, 'd');
    updateActionState(input_state.reset, 'r');
}

void apply_user_input(const sim_config& config, fluid_container& container, InputState& input_state, vector<float>& emission_arr)
{
    if (isKeyPressed('q') || isKeyPressed('\x03')) 
    {
        input_state.quit = true;
    }
    else
    {
        add_wind(config, container, input_state);
        add_sources(config, container, input_state, emission_arr);
    }
}