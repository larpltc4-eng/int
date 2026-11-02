#pragma once

#include "imgui.h"
#include "imgui_internal.h"

// Widgets of the UziClicker menu. Everything is placed at absolute positions inside the current
// window; coordinates passed in are in pixels at 100% scale (see style.hpp) and are multiplied by
// the DPI scale set with SetScale(). Widgets return true when the value changed / on click.
namespace uzi::widgets
{
    void  SetScale(float scale);
    float GetScale();

    // Base (100%) coordinates -> screen coordinates in the current window, rounded to whole pixels.
    float  Px(float v);
    ImVec2 Pos(float x, float y);
    ImRect Rect(float x, float y, float w, float h);

    // Text whose baseline is at `baseline` (base coordinates).
    void  Text(ImFont* font, float x, float baseline, ImU32 color, const char* text);
    // "Lewy przycisk myszy " in white followed by "LPM" in the accent color.
    void  Label(float x, float baseline, const char* text, const char* accent_text);
    float TextWidth(ImFont* font, const char* text); // screen pixels

    bool SidebarButton(const char* label, bool selected, const ImRect& bb);
    bool BindButton(const char* str_id, ImGuiKey* key, const ImRect& bb); // Esc / Backspace clears the bind
    bool Dropdown(const char* str_id, int* current, const char* const items[], int count, const ImRect& bb);
    bool Slider(const char* str_id, int* v, int v_min, int v_max, float track_center_y);
    bool ValueBox(const char* str_id, int* v, int v_min, int v_max, const ImRect& bb, float border, float baseline);
    bool Toggle(const char* str_id, const char* label, bool* v, float label_x, float toggle_x, float baseline);

    const char* KeyName(ImGuiKey key); // "R", "Mysz 4", "?" for no key
    void Particles(const ImRect& area);  // glowing dots floating over the menu
}
