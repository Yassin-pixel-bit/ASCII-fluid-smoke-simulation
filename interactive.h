#pragma once

#include <iostream>
#include "fluid_math.h"
#include "input_state.h"
#include "terminal.h"

void apply_user_input(fluid_container& container, InputState& input_state, std::vector<float>& emission_arr);
void update_input(InputState& input_state);