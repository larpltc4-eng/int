#pragma once

namespace menu
{
    inline bool open = true;

    // Call once after ImGui::CreateContext(): loads the embedded fonts and applies the style.
    void Initialize();

    // Call every frame between ImGui::NewFrame() and ImGui::Render().
    void Render();
}
