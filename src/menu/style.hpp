#pragma once

#include "imgui.h"

// Every size and color here was measured from the reference screenshot (pixel exact).
namespace menu::style
{
    // Layout -----------------------------------------------------------------
    inline constexpr ImVec2 kWindowPos    = { 11.0f, 11.0f };
    inline constexpr ImVec2 kWindowSize   = { 742.0f, 448.0f };

    inline constexpr float kSidebarWidth          = 56.0f;  // collapsed (icons only)
    inline constexpr float kSidebarWidthExpanded  = 160.0f; // after clicking the hamburger
    inline constexpr float kSidebarHeaderHeight   = 59.0f;  // hamburger cell
    inline constexpr float kSidebarItemHeight     = 60.0f;
    inline constexpr float kSidebarIconOffsetY    = 15.0f;  // icon line sits this much above the cell's vertical center
    inline constexpr float kSidebarLabelX         = 56.0f;  // labels start where the collapsed sidebar ends

    inline constexpr float kContentPadding        = 8.0f;   // left/right padding of the content area
    inline constexpr float kTabWidth              = 100.0f;
    inline constexpr float kTabHeight             = 44.0f;  // including the underline
    inline constexpr float kTabLineHeight         = 4.0f;
    inline constexpr float kTabLineRounding       = 1.0f;
    inline constexpr float kTabTextOffsetY        = 13.0f;  // label line top, from the tab's top edge
    inline constexpr float kPageSpacing           = 4.0f;   // gap between tab bar and page content

    inline constexpr float kFooterHeight          = 14.0f;
    inline constexpr float kFooterPaddingX        = 3.0f;
    inline constexpr float kFooterPaddingRightX   = 5.0f;

    inline constexpr float kFontSize              = 17.0f;
    inline constexpr float kFooterFontSize        = 14.0f;
    inline constexpr float kIconFontSize          = 20.0f;

    // Colors -----------------------------------------------------------------
    inline constexpr ImU32 kSidebarBg       = IM_COL32( 30,  29,  35, 255);
    inline constexpr ImU32 kSidebarActiveBg = IM_COL32( 35,  35,  44, 255);
    inline constexpr ImU32 kContentBg       = IM_COL32( 21,  20,  27, 255);
    inline constexpr ImU32 kDark            = IM_COL32( 15,  15,  15, 255); // footer + tab bar line
    inline constexpr ImU32 kAccent          = IM_COL32( 80,   0, 255, 255);
    inline constexpr ImU32 kTabGradientTop  = IM_COL32( 37,  16,  75, 255);
    inline constexpr ImU32 kTabGradientBot  = IM_COL32( 79,   5, 194, 255);

    inline constexpr ImU32 kIcon            = IM_COL32( 74,  77,  82, 255);
    inline constexpr ImU32 kIconHovered     = IM_COL32(150, 152, 158, 255);
    inline constexpr ImU32 kIconActive      = IM_COL32(255, 255, 255, 255);

    inline constexpr ImU32 kTabText         = IM_COL32(199, 199, 199, 255);
    inline constexpr ImU32 kTabTextActive   = IM_COL32(255, 255, 255, 255);
    inline constexpr ImU32 kText            = IM_COL32(255, 255, 255, 255);

    // Applies the palette above to ImGuiStyle so stock widgets match the menu.
    void Apply();
}
