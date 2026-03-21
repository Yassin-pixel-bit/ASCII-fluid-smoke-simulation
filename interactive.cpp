#include "interactive.h"

using namespace std;

void add_wind(fluid_container& container, const InputState& input_state)
{
        int center_x = container.width / 2;
        int center_y = container.height / 2;
        int top_y = 2;

        float wind_force = 500.0f;
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

void add_sources(fluid_container& container, const InputState& input_state, vector<float>& emission_arr)
{
    int center_x = container.width / 2;
    int center_y = container.height / 2;
    int top_y = 2;

    if (input_state.pouring_smoke)
    {
        // Add density
        emission_arr[container.IDX(center_x, top_y)] = 1000.0f;
        emission_arr[container.IDX(center_x + 1, top_y)] = 1000.0f;
        emission_arr[container.IDX(center_x, top_y)] = 1000.0f;

        container.vel_y_prev[container.IDX(center_x, top_y)] = 500.0f;
        container.vel_y_prev[container.IDX(center_x - 1, top_y)] = 500.0f;
        container.vel_y_prev[container.IDX(center_x + 1, top_y)] = 500.0f;
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

void apply_user_input(fluid_container& container, InputState& input_state, vector<float>& emission_arr)
{
    if (isKeyPressed('q') || isKeyPressed('\x03')) 
    {
        input_state.quit = true;
    }
    else
    {
        add_wind(container, input_state);
        add_sources(container, input_state, emission_arr);
    }
}