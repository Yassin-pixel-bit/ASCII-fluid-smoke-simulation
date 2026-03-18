#include "terminal.h"

#ifdef _WIN32
    #include <windows.h>
    #ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
        #define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
    #endif
#else
    #include <sys/ioctl.h>
    #include <unistd.h>
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