#pragma once

struct ImFont;

namespace cipherx::fonts
{
    inline ImFont* regular = nullptr; // Segoe UI 13          - buttons (default font)
    inline ImFont* small   = nullptr; // Segoe UI 12          - "licensed user", status label (drawn at 11.5)
    inline ImFont* title   = nullptr; // Segoe UI Semibold 18 - title
    inline ImFont* mono    = nullptr; // Consolas 15          - console

    // Adds the fonts to io.Fonts. Call once after ImGui::CreateContext() and before the renderer backend
    // builds the font atlas (i.e. before the first NewFrame).
    // On Windows the fonts are loaded from the Windows font folder. Elsewhere (or if a file is missing) embedded
    // replacements are used: Selawik for Segoe UI and JetBrains Mono for Consolas, both given the vertical metrics
    // of the font they replace, so they have the same size and baseline.
    void Load();
}
