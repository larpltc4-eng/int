#pragma once

struct ImFont;

namespace menu::fonts
{
    inline ImFont* regular = nullptr; // Montserrat Medium 17    - page content (default font)
    inline ImFont* tab     = nullptr; // Montserrat SemiBold     - tab/sidebar labels (rasterized at 28, drawn at 17)
    inline ImFont* footer  = nullptr; // Montserrat Medium       - footer (rasterized at 15, drawn at 14)
    inline ImFont* icons   = nullptr; // Font Awesome 5 Solid 20 - sidebar

    // Adds the embedded fonts to io.Fonts. Call once after ImGui::CreateContext()
    // and before the renderer backend builds the font atlas (i.e. before the first NewFrame).
    void Load();
}
