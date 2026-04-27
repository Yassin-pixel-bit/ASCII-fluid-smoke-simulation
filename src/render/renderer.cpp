#include "renderer.h"

using namespace std;

struct frame_emitter
{
    std::string& out;
    bool use_colors;

    // cursor tracking
    int expected_x = -1;
    int expected_y = -1;

    // color tracking
    RGB current_color = {0, 0, 0};
    bool is_colored = false;

    void emit_color(int char_index, char cell_char)
    {
        if (cell_char != render_str[0])
        {
            RGB color = get_theme_color(char_index);
            if (!is_colored || color != current_color)
            {
                out.append(get_theme_ansi(char_index));
                current_color = color;
                is_colored    = true;
            }
        }
        else if (is_colored)
        {
            out.append("\033[0m");
            is_colored = false;
        }
    }

    void emit_cell(int char_index, int target_x, int target_y)
    {
        if (expected_y != target_y || expected_x != target_x)
            fmt::format_to(back_inserter(out), "\033[{};{}H", target_y, target_x);

        char cell_char = render_str[char_index];

        if (use_colors)
            emit_color(char_index, cell_char);

        // Each cell is two characters wide
        out.push_back(cell_char);
        out.push_back(cell_char);

        expected_x = target_x + 2;
        expected_y = target_y;
    }

    void close()
    {
        if (is_colored) out.append("\033[0m");
    }
};

static void check_frame_reset(vector<int>& prev_frame, string& print_string, int frame_size, bool use_colors, bool& last_color_state)
{
    if (prev_frame.size() != (size_t)frame_size || last_color_state != use_colors)
    {
        prev_frame.assign(frame_size, -1);
        print_string.append("\033[2J");
        last_color_state = use_colors;
    }
}

void set_print_string(string &print_string, const vector<float>& grid ,const int TERMINAL_LEN, const int TERMINAL_WIDTH, bool use_colors)
{
    static vector<int> prev_frame;
    static bool last_color_state = use_colors;

    check_frame_reset(prev_frame, print_string, TERMINAL_LEN * TERMINAL_WIDTH, use_colors, last_color_state);

    print_string.clear();

    frame_emitter emitter{print_string, use_colors};

    int grid_stride = TERMINAL_WIDTH + 2;
    int fluid_index = grid_stride + 1; // skip top wall and first left wall
    int frame_idx   = 0;

    for (int y = 0; y < TERMINAL_LEN; y++)
    {
        for (int x = 0; x < TERMINAL_WIDTH; x++)
        {
            float density   = grid[fluid_index++];
            int   char_index = clamp((int)(density * (render_str_len - 1)), 0, render_str_len - 1);

            if (prev_frame[frame_idx] != char_index)
            {
                prev_frame[frame_idx] = char_index;

                // ANSI rows/cols are 1-based; row 1 is the HUD so content starts at row 2.
                // Each cell is 2 terminal columns wide, so multiply x by 2.
                emitter.emit_cell(char_index,
                          (x * 2) + 1, y + 2);
            }

            frame_idx++;
        }
        fluid_index += 2; // jump over right wall and next row's left wall
    }

    emitter.close();
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