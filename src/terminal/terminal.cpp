#include "terminal.h"
#include <cstring>

#ifdef _WIN32
    #include <windows.h>
#else
    #include <sys/ioctl.h>
    #include <unistd.h>
    #include <termios.h>
#endif

int terminal_width = 0;
int terminal_height = 0;

// These functions just return copies of the terminal sizes.
int getTerminalWidth() { return terminal_width; }
int getTerminalHeight() { return terminal_height; }

static bool key_just_pressed[256] = {false};

void initTerminalSize() {
    #ifdef _WIN32
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
        terminal_width = csbi.srWindow.Right - csbi.srWindow.Left + 1;
        terminal_height = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
    #else
        struct winsize w;
        ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
        terminal_width = w.ws_col;
        terminal_height = w.ws_row;
    #endif
}

void enableANSI() {
#ifdef _WIN32
    // Get the output handle for the console
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut == INVALID_HANDLE_VALUE) return;

    // Get the current console mode
    DWORD dwMode = 0;
    if (!GetConsoleMode(hOut, &dwMode)) return;

    // Flip the bit to enable ANSI (Virtual Terminal Processing)
    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hOut, dwMode);
#else
    // Linux and macOS terminals process ANSI natively. 
#endif
}

bool isKeyPressed(char key) {
#ifdef _WIN32
    // Windows queries hardware directly
    int vKey = (key == ' ') ? VK_SPACE : toupper(key);
    return (GetAsyncKeyState(vKey) & 0x8000) != 0;
#else
    return key_just_pressed[(unsigned char)key];
#endif
}

bool isKeyJustPressed(char key) {
#ifdef _WIN32
    return false; 
#else
    return key_just_pressed[(unsigned char)key];
#endif
}

#ifndef _WIN32
static struct termios original_t;
static bool original_saved = false;
static bool input_initialized = false;
#endif

void updateInput() 
{
#ifndef _WIN32
    if (!input_initialized) {
        tcgetattr(STDIN_FILENO, &original_t);
        original_saved = true;
        struct termios t = original_t;

        t.c_lflag &= ~(ICANON | ECHO);

        t.c_cc[VMIN] = 0;
        t.c_cc[VTIME] = 0; 

        tcsetattr(STDIN_FILENO, TCSANOW, &t);
        input_initialized = true;
    }

    std::memset(key_just_pressed, 0, sizeof(key_just_pressed));

    char buf[32];
    int bytes_read;

    while ((bytes_read = read(STDIN_FILENO, buf, sizeof(buf))) > 0) 
    {
        for (int i = 0; i < bytes_read; i++) 
            key_just_pressed[(unsigned char)buf[i]] = true;
    }

#endif
}

void flushTerminalInput() {
#ifdef _WIN32
    // Safely discard all pending console input events on Windows
    FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE));
#else
    // Flush the non-canonical input buffer on Linux/macOS
    tcflush(STDIN_FILENO, TCIFLUSH);
#endif
}

// Implement the restore function
void restoreTerminal() {
#ifndef _WIN32
    if (original_saved) {
        // Push the original settings back to the terminal immediately
        tcsetattr(STDIN_FILENO, TCSANOW, &original_t);
        input_initialized = false;
    }
#endif
}
