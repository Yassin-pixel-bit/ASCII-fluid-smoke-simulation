#include "renderer.h"

using namespace std;

struct run_flusher {
    string& print_string;
    bool use_colors;
    
    RGB current_terminal_color = {0, 0, 0}; 
    bool is_colored = false;

    inline void flush(int run_index, int run_length) {
        if (run_length == 0) return;

        char run_char = render_str[run_index];

        if (use_colors)
        {
            RGB run_color = get_theme_color(run_index);

            if (run_char != render_str[0]) 
            {
                if (!is_colored || run_color != current_terminal_color) 
                {
                    print_string.append(get_theme_ansi(run_index));
                    current_terminal_color = run_color;
                    is_colored = true;
                }
            } 
            else if (is_colored) 
            {
                print_string.append("\033[0m");
                is_colored = false;
            }
        }

        int total_chars = run_length * 2;
        if (total_chars <= 4) 
        {
            print_string.append(total_chars, run_char);
        }
        else 
        {
            print_string.push_back(run_char);
            fmt::format_to(back_inserter(print_string), "\033[{}b", total_chars - 1);
        }
    }

    inline void close() {
        if (is_colored) print_string.append("\033[0m");
    }
};

void set_print_string(string &print_string, const vector<float>& grid ,const int TERMINAL_LEN, const int TERMINAL_WIDTH, bool use_colors)
{
    int grid_stride = TERMINAL_WIDTH + 2;
    // Skip the top boundary wall, and the first left boundary wall
    int fluid_index = grid_stride + 1;

    print_string.clear();

    run_flusher  flusher{print_string, use_colors};

    RGB current_terminal_color = {0, 0, 0}; 
    bool is_colored = false;

    for (int i = 0; i < TERMINAL_LEN; i++)
    {
        int current_run_index = -1; 
        int cell_run_length = 0;

        for (int j = 0; j < TERMINAL_WIDTH; j++)
        {
            float density = grid[fluid_index];
            fluid_index++;

            int max_index = render_str_len - 1;
            int char_index = clamp((int)(density * max_index), 0, max_index);

            if (char_index == current_run_index)
            {
                cell_run_length++;
            }
            else
            {
                flusher.flush(current_run_index, cell_run_length);
                current_run_index = char_index;
                cell_run_length = 1;
            }
        }

        flusher.flush(current_run_index, cell_run_length);

        // Jump over the right wall (+1) and the next row's left wall (+1)
        fluid_index += 2;

        // as long as we are not at the last row add a newline char
        if ((i + 1) != TERMINAL_LEN)
            print_string.push_back('\n');
    }

    flusher.close();
}

bool update_hud(float dt, const string_view theme_name, int term_width, string& out_hud)
{
    static float fps_timer = 1.0f;
    static int frame_count = 0;

    fps_timer += dt;
    frame_count++;

    if (fps_timer >= 1.0f)
    {
        string hud_text = fmt::format("  [Theme: {}]   FPS: {:<4} Controls: [R]reset  [C]clear  [Q]quit", theme_name, frame_count);
        string padded_hud = fmt::format("{:^{}}", hud_text, term_width);
        
        // Dark Gray background and Pure White text
        out_hud = fmt::format("\033[1;1H\033[1m\033[38;5;15m\033[48;5;236m{}\033[0m", padded_hud);

        fps_timer -= 1.0f;
        frame_count = 0;
        return true; 
    }
    return false;
}