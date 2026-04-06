#pragma once

#include <algorithm>
#include <string>
#include <vector>
#include <iterator>
#include <string_view>
#include "themes.h"

constexpr std::string_view render_str = R"( .`'-_,:~=;!*+<>\/|?#@)";
constexpr int render_str_len = render_str.size();

void set_print_string(std::string &print_string, const std::vector<float>& grid ,const int TERMINAL_LEN, const int TERMINAL_WIDTH, bool use_colors);
std::string get_fps_overlay(float dt);