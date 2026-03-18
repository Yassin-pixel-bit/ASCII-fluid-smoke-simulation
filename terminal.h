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
