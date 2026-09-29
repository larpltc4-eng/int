#pragma once

#include "imgui.h"

// Dear ImGui menu that recreates the reference screenshot 1:1: an empty black title bar, two rows of
// four black tiles (the selected one is white) and the "Skin" page under them. The window background is
// translucent, so whatever is rendered behind it shows through.
//
// This is only the interface: it holds the values of its widgets and does nothing else.
namespace tilemenu
{
    enum Tab
    {
        Tab_Aimbot, Tab_Esp, Tab_Visuals, Tab_Misc,
        Tab_Skin, Tab_Thirdperson, Tab_Effects, Tab_Setup,
        Tab_Count
    };

    inline constexpr int kBuddyMax = 865; // upper end of the buddy slider

    struct State
    {
        int tab = Tab_Skin;

        // "Skin" page (values shown in the reference).
        bool collection_skin_changer = true;
        bool eject_shells = true;
        bool buy_binds = true;
        bool buddy = true;
        int buddy_index = 625; // 0 .. kBuddyMax

        // Set for one frame when "unlock all skins" is clicked; read it in your own code after Render().
        bool unlock_all_skins_clicked = false;
    };

    inline State state;
    inline bool open = true;
    inline int toggle_key = ImGuiKey_Insert; // ImGuiKey that opens/closes the menu

    // Your own page for a tab. A null entry uses the built-in one: the Skin page from the reference, and an empty
    // page for every other tab (the reference only shows Skin). The function is called inside the menu window,
    // right under the tabs.
    using PageFn = void (*)();
    inline PageFn pages[Tab_Count] = {};

    // Name shown under the buddy slider. Default: "V25A3: Radiant Buddy" for 625 (as in the reference), "Buddy #N" otherwise.
    using BuddyNameFn = const char* (*)(int index);
    inline BuddyNameFn buddy_name = nullptr;

    // Call once after ImGui::CreateContext(), before the renderer backend builds the font atlas (before the first NewFrame).
    // Adds Roboto Bold (32 px em) to the atlas (does not change ImGui's default font or style).
    void Initialize();

    // Call every frame between ImGui::NewFrame() and ImGui::Render(). The menu pushes its own style and font and pops them again.
    void Render();

    // Optional, for demos: paints a red-to-dark gradient on the background draw list so the translucent window is visible.
    void DrawDemoBackdrop();
}
