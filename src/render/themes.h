#pragma once

#include <fmt/core.h>
#include <fmt/color.h>
#include <fmt/format.h>
#include <vector>

struct RGB {
    // min 0, max 255
    uint8_t r;
    uint8_t g;
    uint8_t b;

    bool operator != (const RGB& other) const 
    {
        return r != other.r || g != other.g || b != other.b;
    }
};

struct color_stop {
    float pos; // where the color peaks
    RGB color; // the color
};

using gradient_theme = std::vector<color_stop>;

void init_selected_theme(int choice, int steps);
void print_theme_menu();
RGB get_theme_color(int index);
std::string_view get_theme_ansi(int index);
int get_themes_max();
std::string get_theme_name(int idx);