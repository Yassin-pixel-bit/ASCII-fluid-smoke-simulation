#include "interactive.h"

using namespace std;

void add_wind(const sim_config& config, fluid_container& container, const InputState& input_state)
{
        int center_x = container.width / 2;
        int center_y = container.height / 2;
        int top_y = 2;

        float wind_force = config.wind_force;
        if (input_state.wind_w) {
            container.vel_y_prev[container.IDX(center_x, container.height - 2)] = -wind_force;
        }
        if (input_state.wind_s) {
            container.vel_y_prev[container.IDX(center_x, top_y)] = wind_force;
        }
        if (input_state.wind_a) {
            container.vel_x_prev[container.IDX(container.width - 2, center_y)] = -wind_force;
        }
        if (input_state.wind_d) {
            container.vel_x_prev[container.IDX(top_y, center_y)] = wind_force;
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

        int cells_in_brush = 1;
        if (config.dist_fluid && radius > 0)
        {
            cells_in_brush = 0;
            for (int i = -radius; i <= radius; i++) {
                for (int j = -radius; j <= radius; j++) {
                    if (in_brush_bound(i, j, radius)) {
                        cells_in_brush++;
                    }
                }
            }
            amount = amount / cells_in_brush;
            push_dir = push_dir / cells_in_brush;
        }

        for (int i = -radius; i <= radius; i++) 
        {
            for (int j = -radius; j <= radius; j++) 
            {
                // Check if the current offset is inside the circle
                if (in_brush_bound(i, j, radius)) 
                {
                    int target_x = fluid_x + j;
                    int target_y = fluid_y + i;

                    // Keep the brush strictly inside the grid boundaries
                    if (target_x > 0 && target_x < container.width - 1 &&
                        target_y > 0 && target_y < container.height - 1) 
                    {
                        emission_arr[container.IDX(target_x, target_y)] += amount;
                        container.vel_y_prev[container.IDX(target_x, target_y)] += push_dir;
                    }
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