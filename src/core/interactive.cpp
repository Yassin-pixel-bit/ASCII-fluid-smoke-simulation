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

void add_sources(const sim_config& config, fluid_container& container, const InputState& input_state, vector<float>& emission_arr)
{
    int fluid_x = 2 + (int)((container.width - 3) * config.spawn_x);
    int fluid_y = 2 + (int)((container.height - 3) * config.spawn_y);

    if (input_state.pouring_smoke)
    {
        float amount = config.fluid_amount;
        float push = config.spawn_push;

        // Add density
        emission_arr[container.IDX(fluid_x, fluid_y)] = amount;
        emission_arr[container.IDX(fluid_x + 1, fluid_y)] = amount;
        emission_arr[container.IDX(fluid_x - 1, fluid_y)] = amount;

        container.vel_y_prev[container.IDX(fluid_x, fluid_y)] = config.spawn_y < 0.5 ? push : -push;
        container.vel_y_prev[container.IDX(fluid_x - 1, fluid_y)] = config.spawn_y < 0.5 ? push : -push;
        container.vel_y_prev[container.IDX(fluid_x + 1, fluid_y)] = config.spawn_y < 0.5 ? push : -push;
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