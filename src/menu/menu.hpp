#pragma once

#include "imgui.h"

namespace menu
{
    inline bool open = true;
    inline int toggle_key = ImGuiKey_Insert; // ImGuiKey that opens/closes the menu (editable in the menu)

    // Call once after ImGui::CreateContext(): loads the embedded fonts and applies the style.
    void Initialize();

    // Call every frame between ImGui::NewFrame() and ImGui::Render().
    void Render();
}
