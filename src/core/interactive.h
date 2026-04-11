#pragma once

#include <iostream>
#include "fluid_math.h"
#include "input_state.h"
#include "terminal.h"
#include "settings.h"

void apply_user_input(const sim_config& config, fluid_container& container, const InputState& input_state, std::vector<float>& emission_arr);
void update_input(InputState& input_state);