#include "terminal.h"
#include <cstring>

#ifdef _WIN32
    #include <windows.h>
#else
    #include <sys/ioctl.h>
    #include <unistd.h>
    #include <termios.h>
    #include <poll.h>
#endif

int terminal_width = 0;
int terminal_height = 0;

// These functions just return copies of the terminal sizes.
int getTerminalWidth() { return terminal_width; }
int getTerminalHeight() { return terminal_height; }

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

static int key_timeouts[256] = {0};
static bool key_just_pressed[256] = {false};

#ifndef _WIN32
static struct termios original_t;
static bool original_saved = false;
#endif

void updateInput() 
{
#ifndef _WIN32
    static bool init = false;
    if (!init) {
        tcgetattr(STDIN_FILENO, &original_t);
        original_saved = true;
        struct termios t = original_t;
        t.c_lflag &= ~(ICANON | ECHO);
        tcsetattr(STDIN_FILENO, TCSANOW, &t);
        init = true;
    }

    for (int i = 0; i < 256; i++) {
        if (key_timeouts[i] > 0) key_timeouts[i]--;
    }

    std::memset(key_just_pressed, 0, sizeof(key_just_pressed));

    struct pollfd pfd = { STDIN_FILENO, POLLIN, 0 };
    
    while (poll(&pfd, 1, 0) > 0) {
        char buf;
        if (read(STDIN_FILENO, &buf, 1) > 0) {
            unsigned char u_buf = (unsigned char)buf;
            
            // If the timeout was 0, this is a brand new press
            if (key_timeouts[u_buf] == 0) {
                key_just_pressed[u_buf] = true;
            }
            
            // Reset the standard buffer lifespan
            key_timeouts[u_buf] = 10; 
        }
    }
#endif
}

// Implement the restore function
void restoreTerminal() {
#ifndef _WIN32
    if (original_saved) {
        // Push the original settings back to the terminal immediately
        tcsetattr(STDIN_FILENO, TCSANOW, &original_t);
    }
#endif
}

bool isKeyPressed(char key) {
#ifdef _WIN32
    // Windows queries hardware directly
    int vKey = (key == ' ') ? VK_SPACE : toupper(key);
    return (GetAsyncKeyState(vKey) & 0x8000) != 0;
#else
    // Linux checks if the key still has a positive timeout lifespan
    return key_timeouts[(unsigned char)key] > 0;
#endif
}

bool isKeyJustPressed(char key) {
#ifdef _WIN32
    return false; 
#else
    return key_just_pressed[(unsigned char)key];
#endif
}