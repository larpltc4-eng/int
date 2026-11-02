#pragma once

#include "imgui.h"

// All sizes are in pixels at 100% DPI scale and were measured from the reference screenshot
// (the window size, 624x415, is uzi::kWidth / uzi::kHeight in menu.hpp).
// The screenshot is a frame of a video: it is scaled to 2/3 and its colors are squeezed into the
// 16-235 "video range" (the white Windows title bar is 235 there), both were undone here.
namespace uzi::style
{
    // Fonts: Poppins, sizes are em sizes (CSS / GDI "font-size" in px) as measured. ImGui sizes a font by
    // ascender - descender instead and they are rounded to whole pixels there, see fonts.cpp
    // (ImGui sizes: 20, 23, 32 and 19 px at 100%).
    inline constexpr float kTextSize    = 14.5f;  // Regular: labels, buttons, values
    inline constexpr float kSidebarSize = 16.75f; // Regular: sidebar buttons
    inline constexpr float kTitleSize   = 22.9f;  // Medium: "UziClicker"
    inline constexpr float kToggleSize  = 13.4f;  // Bold: ON / OFF

    // Sidebar -------------------------------------------------------------------------------
    inline constexpr ImVec2 kTitlePos          = { 17.0f, 35.0f };  // x, baseline
    inline constexpr float  kSidebarX          = 11.0f;
    inline constexpr float  kSidebarY          = 51.0f;              // top of the first button
    inline constexpr float  kSidebarPitch      = 45.71f;             // distance between button tops
    inline constexpr ImVec2 kSidebarButtonSize = { 124.0f, 35.0f };
    inline constexpr float  kSidebarBaseline   = 24.0f;              // label baseline below the button top

    // Content -------------------------------------------------------------------------------
    inline constexpr float kColumn1         = 151.0f; // labels, bind buttons, "Gardaowanie MS"
    inline constexpr float kColumn2         = 365.0f; // "Tryb klikania", dropdowns
    inline constexpr float kSectionPitch    = 128.0f; // LPM section -> PPM section
    inline constexpr float kHeaderBaseline  = 33.0f;  // "Lewy przycisk myszy LPM"

    inline constexpr float kRowTop          = 49.0f;  // bind button + dropdown
    inline constexpr float kRowHeight       = 26.0f;
    inline constexpr float kBindWidth       = 155.0f;
    inline constexpr float kBindBaseline    = 17.0f;  // below the button top
    inline constexpr float kDropdownWidth   = 124.0f;
    inline constexpr float kDropdownRounding = 10.0f;
    inline constexpr float kDropdownTextX   = 9.0f;
    inline constexpr float kDropdownBaseline = 17.0f;
    inline constexpr float kArrowRight      = 18.0f;  // arrow center, from the dropdown's right edge
    inline constexpr float kArrowTop        = 10.0f;  // from the dropdown top
    inline constexpr ImVec2 kArrowSize      = { 7.6f, 4.5f };
    inline constexpr float kItemHeight      = 26.0f;  // dropdown list items

    inline constexpr float kSliderY         = 96.0f;  // track center of the first (MIN CPS) row
    inline constexpr float kSliderPitch     = 28.0f;  // MIN CPS -> MAX CPS
    inline constexpr float kSliderBaseline  = 7.0f;   // label/value baseline below the track center
    inline constexpr float kTrackX0         = 223.0f;
    inline constexpr float kTrackX1         = 469.0f;
    inline constexpr float kTrackHeight     = 6.0f;
    inline constexpr float kKnobRadius      = 6.4f;
    inline constexpr float kValueBoxX       = 483.0f;
    inline constexpr ImVec2 kValueBoxSize   = { 52.0f, 21.0f };
    inline constexpr float kValueBoxTop     = -9.0f;  // relative to the track center

    inline constexpr float  kBottomBaseline = 288.0f; // "Gardaowanie MS", "Szczegółowe opcje"
    inline constexpr ImVec4 kMsBox          = { 151.0f, 298.0f, 62.0f, 32.0f }; // x, y, w, h
    inline constexpr float  kMsBaseline     = 21.0f;  // below the box top
    inline constexpr float  kOptionsHeaderX = 309.0f;
    inline constexpr float  kOptionsX       = 313.0f;
    inline constexpr float  kOptionsY       = 314.0f; // baseline of the first option
    inline constexpr float  kOptionsPitch   = 23.4f;
    inline constexpr float  kToggleX        = 493.0f;

    inline constexpr float kBorder          = 2.0f;   // sidebar, bind and ms box outlines
    inline constexpr float kValueBoxBorder  = 1.5f;

    // Colors --------------------------------------------------------------------------------
    inline constexpr ImU32 kBackground     = IM_COL32(  0,   0,   0, 255);
    inline constexpr ImU32 kText           = IM_COL32(255, 255, 255, 255);
    inline constexpr ImU32 kAccent         = IM_COL32(  0, 187, 255, 255); // outlines, blue words
    inline constexpr ImU32 kAccentFill     = IM_COL32(  0, 169, 233, 255); // selected sidebar button
    inline constexpr ImU32 kAccentHover    = IM_COL32(  0, 169, 233,  46); // fill of hovered outline buttons
    inline constexpr ImU32 kAccentActive   = IM_COL32(  0, 169, 233,  80);
    inline constexpr ImU32 kSlider         = IM_COL32(  0, 187, 255, 255); // slider fill + knob
    inline constexpr ImU32 kSliderRail     = IM_COL32( 60,  62,  62, 255);
    inline constexpr ImU32 kDropdown       = IM_COL32(255, 255, 255, 255);
    inline constexpr ImU32 kDropdownHover  = IM_COL32(232, 232, 232, 255);
    inline constexpr ImU32 kDropdownText   = IM_COL32(  0,   0,   0, 255);
    inline constexpr ImU32 kItemHover      = IM_COL32(  0, 169, 233,  40);
    inline constexpr ImU32 kOff            = IM_COL32(255,  75,  70, 255);
    inline constexpr ImU32 kOn             = IM_COL32( 40, 205,  90, 255);
    inline constexpr ImU32 kParticle       = IM_COL32(  0, 187, 255, 255);
}
