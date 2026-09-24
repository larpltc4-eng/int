#pragma once

#include "imgui.h"

namespace menu::widgets
{
    // Smoothly moves a per-id value towards `target` (stored in the current window's state storage).
    float Animate(ImGuiID id, float target, float speed = 14.0f);
    ImU32 LerpColor(ImU32 a, ImU32 b, float t);

    // Sidebar entry: icon centered in the collapsed column, label shown while the sidebar is expanded.
    // Pass label == nullptr for the hamburger header. Returns true when clicked.
    bool SidebarButton(const char* id, const char* icon, const char* label, bool selected, const ImVec2& size, float label_alpha);

    // 100x44 top tab with the purple gradient + underline when selected. Returns true when clicked.
    bool Tab(const char* label, bool selected);

    // Checkbox whose box is invisible until hovered/checked (as in the reference menu).
    bool Checkbox(const char* label, bool* v);
}
