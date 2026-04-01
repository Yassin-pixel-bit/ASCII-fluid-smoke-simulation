#include "themes.h"

std::vector<RGB> palette;

inline uint8_t lerp(uint8_t start, uint8_t end, float prct)
{
    return static_cast<uint8_t>(start + (end - start) * prct);
}

RGB evaluate_gradient(const gradient_theme& theme, float density)
{
    // clamp to the first or last color if out of bounds
    if (density <= theme.front().pos) return theme.front().color;
    if (density >= theme.back().pos) return theme.back().color;

    for (int i = 0; i < theme.size() - 1; i++)
    {
        const auto& start_stop = theme[i];
        const auto& end_stop = theme[i + 1];

        bool in_range = density >= start_stop.pos && density <= end_stop.pos; 
        if (in_range)
        {
            float range = end_stop.pos - start_stop.pos;
            float local_prct = (density - start_stop.pos) / range;

            RGB result;
            result.r = lerp(start_stop.color.r, end_stop.color.r, local_prct);
            result.g = lerp(start_stop.color.g, end_stop.color.g, local_prct);
            result.b = lerp(start_stop.color.b, end_stop.color.b, local_prct);

            return result;
        }
    }

    return {255,255,255};
}

void init_theme(const gradient_theme& theme, int steps)
{
    palette.clear();
    palette.reserve(steps);

    for (int i = 0; i < steps; i++) 
    {
        float density = (float)i / (float)(steps - 1);
        palette.push_back(evaluate_gradient(theme, density));
    }
}

RGB get_theme_color(int index)
{
    return palette[index];
}