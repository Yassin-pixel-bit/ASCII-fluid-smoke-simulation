#include "fluid_math.h"
#include "settings.h"
#include <math.h>

fluid_container::fluid_container(int Height, int Width, float DT)
    : height(Height), width(Width), dt(DT)
{
    int size = (height + 2) * (width + 2);

    dens.resize(size);
    dens_prev.resize(size);
    vel_x.resize(size);
    vel_y.resize(size);
    vel_x_prev.resize(size);
    vel_y_prev.resize(size);

    safety_buffer = 0;
    reset_bounds();
}

void fluid_container::initialize_simulation(float max_wind_force)
{
    float scaled_dt = MAX_DT * std::max(width, height);

    safety_buffer = std::ceil(max_wind_force * scaled_dt) + 1;
}

int fluid_container::find_y_bound(int y_start, int y_end, int step) const
{
    // Scanning downwards
    if (step > 0)
    {

        for (int y = y_start; y <= y_end; y++)
            for (int x = min_x; x <= max_x; x++)
                if (dens[IDX(x, y)] > threshold)
                    return y;

    } else // Scanning upwards
    {
        for (int y = y_start; y >= y_end; y--)
            for (int x = min_x; x <= max_x; x++)
                if (dens[IDX(x, y)] > threshold)
                    return y;
    }

    // the grid is empty
    return -1;
}

int fluid_container::find_x_bound(int y_start, int y_end, int x_start, int current_best, int step) const
{
    for (int y = y_start; y <= y_end; y++)
    {
        // Scanning inwards from the Left
        if (step > 0)
        {
            for (int x = x_start; x < current_best; x++)
            {
                if (dens[IDX(x, y)] > threshold)
                {
                    current_best = x;
                    break;
                }
            }
        } else
        { // Scanning inwards from the Right
            for (int x = x_start; x > current_best; x--)
            {
                if (dens[IDX(x, y)] > threshold)
                {
                    current_best = x;
                    break;
                }
            }
        }
    }
    return current_best;
}

void fluid_container::update_bounds()
{
    // Find Top Edge
    int tight_min_y = find_y_bound(min_y, max_y, 1);

    // If the screen is totally empty, collapse the box to 0
    if (tight_min_y == -1)
    {
        reset_bounds();
        return;
    }

    // Find Bottom Edge
    int tight_max_y = find_y_bound(max_y, tight_min_y, -1);
    // Find Left Edge
    int tight_min_x = find_x_bound(tight_min_y, tight_max_y, min_x, max_x, 1);
    // Find Right Edge
    int tight_max_x = find_x_bound(tight_min_y, tight_max_y, max_x, min_x, -1);

    min_x = std::max(1, tight_min_x - safety_buffer);
    max_x = std::min(width, tight_max_x + safety_buffer);
    min_y = std::max(1, tight_min_y - safety_buffer);
    max_y = std::min(height, tight_max_y + safety_buffer);
}