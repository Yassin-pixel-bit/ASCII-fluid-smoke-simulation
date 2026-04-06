#pragma once
#include "themes.h"

/*
    We use a std::vector here instead of a std::map. 
    Because the user selects a theme via a numeric menu, a vector allows us to 
    route the choice directly to the index in O(1) time. 
    Using a map would require either forcing the user to type string names 
    (introducing case-sensitivity/spelling bugs) or writing clunky O(N) 
    iterator logic to traverse the map by a numeric index (Which I also don't know how to do :) ).
*/

struct theme_def {
    std::string name;
    gradient_theme data;
};

// Themes
using themes = std::vector<theme_def>;

const themes theme_registry = {
    {
        "Cyberpunk", {
            {0.0f, {60, 0, 120}}, // Dark Purple | #3C0078
            {0.15f, {150, 0, 255}}, // Electric Purple | #9600FF
            {0.50f, {255, 0, 128}}, // Hot Pink | #FF0080
            {1.0f, {0, 255, 255}} // Cyan | #00FFFF
        }
    },
    {
        "Fire & Magma", {
            {0.0f, {100, 10, 0}}, // Smoldering Red | #640A00
            {0.3f, {200, 40, 0}}, // Dark Orange-Red | #C82800
            {0.6f, {255, 120, 0}}, // Bright Orange | #FF7800
            {0.85f, {255, 255, 0}}, // Pure Yellow | #FFFF00
            {1.0f, {255, 255, 150}} // Pale Yellow | #FFFF96
        }
    },
    {
        "Uranium Core", {
            {0.0f, {90, 210, 0}}, // Vibrant Green | #5AD200
            {0.4f, {150, 235, 75}}, // Lime Green | #96EB4B
            {0.7f, {210, 250, 100}}, // Pale Yellow-Green | #D2FA64
            {1.0f, {220, 255, 30}} // Blinding Neon Yellow | #DCFF1E
        }
    },
    {
        "Arcane Frost", {
            {0.0f, {40, 40, 110}},       // Deep Indigo | #28286E
            {0.4f, {50, 100, 220}},      // Frost Blue | #3264DC
            {0.7f, {140, 200, 255}},     // Ice Cyan | #8CC8FF
            {1.0f, {230, 240, 250}}      // Pale Ice | #E6F0FA
        }
    },
    {
        "Vaporwave", {
            {0.0f, {70, 0, 140}}, // Deep Violet | #46008C
            {0.3f, {150, 0, 200}}, // Bright Purple | #9600C8
            {0.6f, {255, 0, 128}}, // Hot Pink | #FF0080
            {0.85f, {255, 120, 0}}, // Sunset Orange | #FF7800
            {1.0f, {255, 220, 20}} // Golden Yellow | #FFDC14
        }
    },
    {
        "Synth Dusk", {
            {0.0f, {255, 30, 110}}, // Neon Pink | #FF1E6E
            {0.5f, {150, 50, 255}}, // Electric Violet | #9632FF
            {1.0f, {70, 200, 255}} // Sky Blue | #46C8FF
        }
    },
    {
        "Pastel Sunrise", {
            {0.0f, {255, 130, 230}}, // Pastel Pink | #FF82E6
            {0.35f, {90, 215, 220}}, // Teal | #5AD7DC
            {0.65f, {150, 240, 160}}, // Soft Mint Green | #96F0A0
            {0.85f, {250, 245, 105}}, // Soft Yellow | #FAF569
            {1.0f, {255, 255, 140}} // Pale Bright Yellow | #FFFF8C
        }
    },
    {
        "Phantom Smoke", {
            {0.0f, {40, 40, 40}},        // Dark Charcoal | #282828
            {0.4f, {120, 120, 120}},     // Mid Gray | #787878
            {0.7f, {200, 200, 200}},     // Light Gray | #C8C8C8
            {1.0f, {255, 255, 255}}      // Pure White | #FFFFFF
        }
    }
};