#pragma once

/**
 * @brief a getter for the terminal width
 * @return int the terminal width
*/
int getTerminalWidth();
/**
 * @brief a getter for the terminal height
 * @return int the terminal height
*/
int getTerminalHeight();
/**
 * @brief calculates the terminal diminsions
*/
void initTerminalSize();
/**
 * @brief Enables ANSI codes in terminal
*/
void enableANSI();

/**
 * @brief Checks if a specific key is currently pressed (non-blocking)
 * @param key The character to check
 * @return true if pressed, false otherwise
*/
bool isKeyPressed(char key);

void updateInput();


/**
 * @brief Restores the terminal to its original state (canonical mode + echo)
*/
void restoreTerminal();

void flushTerminalInput();

bool isKeyJustPressed(char key);

/**
 * @brief Updates a boolean state based on the OS-specific input method (Hold vs Toggle)
 * @param state The boolean flag to update
 * @param key The character to check
 */
inline void updateActionState(bool& state, char key) {
#ifdef _WIN32
    state = isKeyPressed(key);
#else
    if (isKeyJustPressed(key)) state = !state;
#endif
}
