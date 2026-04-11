#pragma once

struct InputState {
    bool pouring_smoke = false;
    bool wind_w = false;
    bool wind_a = false;
    bool wind_s = false;
    bool wind_d = false;
    bool quit = false;
    bool reset = false;
    bool clear = false;

    inline void reset_state()
    {
        pouring_smoke = false;
        wind_w = false;
        wind_a = false;
        wind_s = false;
        wind_d = false;
        quit = false;
        reset = false;
        clear = false;
    }
};