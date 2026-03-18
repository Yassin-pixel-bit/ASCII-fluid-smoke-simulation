#include "test_functions.h"

void animateGrid(std::vector<char>& grid, int width, int height) {
    // Static initialization keeps the RNG state across frame calls
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> char_dist(33, 126); // Printable ASCII characters
    static std::uniform_int_distribution<> chance_dist(0, 100);

    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            // 2% chance to change a character each frame to simulate noise
            if (chance_dist(gen) < 2) { 
                grid[(i * width) + j] = static_cast<char>(char_dist(gen));
            }
        }
    }
}