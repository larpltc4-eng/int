#pragma once

struct ImFont;

namespace uzi::fonts
{
    inline ImFont* text    = nullptr; // Poppins Regular - labels, buttons, values (default font)
    inline ImFont* sidebar = nullptr; // Poppins Regular, larger - sidebar buttons
    inline ImFont* title   = nullptr; // Poppins Medium - "UziClicker"
    inline ImFont* toggle  = nullptr; // Poppins Bold - ON / OFF

    // Clears io.Fonts and adds the embedded fonts at the given DPI scale.
    // The renderer backend has to (re)create its font texture afterwards.
    void Load(float scale);
}
