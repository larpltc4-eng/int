#pragma once

#include "imgui.h"

namespace menu::widgets
{
    // Smoothly moves a per-id value towards `target` (stored in the current window's state storage).
    float Animate(ImGuiID id, float target, float speed = 14.0f);
    ImU32 LerpColor(ImU32 a, ImU32 b, float t);

    // Menu chrome ------------------------------------------------------------------

    // Sidebar entry: icon centered in the collapsed column, label shown while the sidebar is expanded.
    // Pass label == nullptr for the hamburger header. Returns true when clicked.
    bool SidebarButton(const char* id, const char* icon, const char* label, bool selected, const ImVec2& size, float label_alpha);

    // 100x44 top tab with the purple gradient + underline when selected. Returns true when clicked.
    bool Tab(const char* label, bool selected);

    // Page content -----------------------------------------------------------------
    // All widgets take the full width of the current panel/column and return true when the value changed
    // (Button: when clicked).

    // Rounded panel with a title bar. A size component <= 0 means "remaining space" (like ImGui::BeginChild).
    // Always call EndPanel(), even when the panel is clipped. Panels can be placed side by side with SameLine().
    void BeginPanel(const char* title, const ImVec2& size = ImVec2(0.0f, 0.0f));
    void EndPanel();

    bool Checkbox(const char* label, bool* v);
    bool Toggle(const char* label, bool* v);
    bool SliderFloat(const char* label, float* v, float v_min, float v_max, const char* format = "%.1f");
    bool SliderInt(const char* label, int* v, int v_min, int v_max, const char* format = "%d");
    bool Combo(const char* label, int* current, const char* const items[], int items_count);
    bool MultiCombo(const char* label, bool* selected, const char* const items[], int items_count);
    bool ColorEdit(const char* label, float col[4], bool alpha = true);
    bool Keybind(const char* label, int* key); // key is an ImGuiKey (ImGuiKey_None = unbound, Escape clears)
    bool Button(const char* label, float width = 0.0f);
}
