#pragma once

#include "imgui.h"

// UziClicker menu (Dear ImGui 1.91 or 1.92+). By default it fills the whole display area, so it is
// the only thing drawn in its own native window: see main_dx11.cpp (Win32 + DirectX 11) and
// main_glfw.cpp (GLFW + OpenGL 3). Render(false) draws it as a movable window inside an existing app.
//
//     ImGui::CreateContext();
//     uzi::Setup(dpi_scale);   // fonts + style, before the renderer backend creates the font texture
//     ...
//     ImGui::NewFrame();
//     uzi::Render();           // every frame
//     ImGui::Render();
namespace uzi
{
    inline constexpr const char* kTitle = "UziClicker"; // window title and sidebar header

    // Client area size at 100% DPI scale, multiply by the DPI scale passed to Setup().
    inline constexpr float kWidth  = 624.0f;
    inline constexpr float kHeight = 415.0f;

    enum class Page : int { Konto, Makro, Klepa, Bindy, Kopanie, Dodatki, Ustawienia, Nowosci, Count };

    enum class ClickMode : int { Hold, Click }; // "Trzymanie", "Kliknięcie"

    // One mouse button section of the "Makro" page.
    struct ButtonSettings
    {
        ImGuiKey  bind;    // ImGuiKey_None = not set ("Bind: ?")
        ClickMode mode;
        int       min_cps;
        int       max_cps;
    };

    struct Settings
    {
        ButtonSettings left  = { ImGuiKey_R,    ClickMode::Hold,  13, 15 }; // "Lewy przycisk myszy LPM"
        ButtonSettings right = { ImGuiKey_None, ClickMode::Click, 25, 35 }; // "Prawy przycisk myszy PPM"
        int  block_ms      = 300;   // "Gardaowanie MS"
        bool auto_block    = false; // "Automatyczna garda"
        bool w_tap         = false; // "Wuwuwanie (W-tap)"
        bool sticky_toggle = false; // "Przełącznie stałe"
        bool particles     = true;  // floating glowing dots over the menu
    };

    // CPS slider ranges (the screenshot's knob positions match 5-30 for LPM and 5-50 for PPM).
    inline constexpr int kLeftCpsMin  = 5, kLeftCpsMax  = 30;
    inline constexpr int kRightCpsMin = 5, kRightCpsMax = 50;
    inline constexpr int kBlockMsMin  = 0, kBlockMsMax  = 9999;

    // Loads the fonts for the given DPI scale (1.0 = 100%). It clears io.Fonts first, so call it before
    // adding other fonts; the menu's text font becomes ImGui's default font. Can be called again (e.g. on
    // WM_DPICHANGED), the renderer backend then has to rebuild its font texture. The global ImGuiStyle is
    // not touched: Render() pushes everything it needs.
    void Setup(float dpi_scale = 1.0f);

    // fill_display = true:  covers the whole io.DisplaySize (the menu is the native window's content).
    // fill_display = false: a movable 624x415 (x DPI scale) ImGui window, drag it by its background.
    void Render(bool fill_display = true);

    Settings& GetSettings();
    Page&     CurrentPage();
}
