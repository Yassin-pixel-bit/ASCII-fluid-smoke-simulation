#pragma once

#include <iostream>
#include <vector>

// Macros

// Convert 2D coordinates (x, y) to 1D array index for a grid of size (n+2) x (n+2)
#define IDX(x, y, stride) ((y) * (stride + 2) + (x))