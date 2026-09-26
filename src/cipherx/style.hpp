#pragma once

#include "imgui.h"

// Every size, position and color here was measured from the reference screenshot.
// Positions are relative to the top-left corner of the window.
namespace cipherx::style
{
    // Window -----------------------------------------------------------------
    inline constexpr ImVec2 kWindowSize         = { 840.0f, 600.0f };
    inline constexpr float  kWindowRounding     = 10.0f;  // only the border is rounded, the background fills the corners
    inline constexpr float  kTopBarHeight       = 72.0f;  // area that drags the window

    // Header
    inline constexpr ImVec2 kLicensePos         = { 24.0f, 35.0f };
    inline constexpr float  kTitleY             = 34.0f;  // the title is centered horizontally
    inline constexpr ImVec2 kCloseCenter        = { 819.0f, 43.0f };
    inline constexpr float  kCloseHalfSize      = 5.0f;   // the X is 10x10
    inline constexpr float  kCloseThickness     = 1.6f;
    inline constexpr float  kCloseHitSize       = 28.0f;

    // Rings in the top-right corner (a snapshot of concentric circles, clipped by the window)
    inline constexpr ImVec2 kRingCenter         = { 794.0f, 28.0f };
    inline constexpr float  kRingRadii[]        = { 150.0f, 224.0f, 300.0f, 378.0f };
    inline constexpr int    kRingSegments       = 96;
    inline constexpr float  kRingThickness      = 1.4f;

    // Content column (status, console, buttons)
    inline constexpr float  kContentX           = 170.0f;
    inline constexpr float  kContentWidth       = 500.0f;

    inline constexpr ImVec2 kStatusDot          = { 175.0f, 166.0f }; // center
    inline constexpr float  kStatusDotRadius    = 3.0f;
    inline constexpr int    kStatusDotSegments  = 12;
    inline constexpr ImVec2 kStatusTextPos      = { 185.0f, 160.0f };

    inline constexpr float  kConsoleY           = 180.0f;
    inline constexpr float  kConsoleHeight      = 200.0f;
    inline constexpr float  kConsoleRounding    = 12.0f;
    inline constexpr ImVec2 kConsolePadding     = { 8.0f, 6.0f };
    inline constexpr float  kConsoleLineSpacing = 2.0f;

    inline constexpr float  kButtonsY           = 402.0f;
    inline constexpr float  kButtonHeight       = 44.0f;
    inline constexpr float  kButtonSpacing      = 12.0f;  // the three buttons share the content width
    inline constexpr float  kButtonRounding     = 10.0f;

    // Fonts (pixel sizes as passed to ImGui)
    inline constexpr float  kFontSize           = 13.0f;  // Segoe UI - buttons
    inline constexpr float  kSmallFontSize      = 12.0f;  // Segoe UI - "licensed user"
    inline constexpr float  kStatusFontSize     = 11.5f;  //            status label, drawn with the 12 px font
    inline constexpr float  kTitleFontSize      = 18.0f;  // Segoe UI Semibold
    inline constexpr float  kMonoFontSize       = 15.0f;  // Consolas - console

    // Colors -----------------------------------------------------------------
    inline constexpr ImU32 kWindowBg       = IM_COL32( 17,  17,  22, 255);
    inline constexpr ImU32 kBorder         = IM_COL32(255, 255, 255,  12); // window, console and button borders
    inline constexpr ImU32 kTitle          = IM_COL32(194, 194, 204, 255);
    inline constexpr ImU32 kLicense        = IM_COL32(129, 129, 139, 255);
    inline constexpr ImU32 kClose          = IM_COL32(121, 121, 131, 255);
    inline constexpr ImU32 kCloseHovered   = IM_COL32(194, 194, 204, 255);
    inline constexpr ImU32 kRingColors[]   = { IM_COL32(100, 112, 160, 87), IM_COL32(100, 112, 160, 65),
                                               IM_COL32(100, 112, 160, 47), IM_COL32(100, 112, 160, 31) };

    inline constexpr ImU32 kStatusRunning  = IM_COL32(143, 184, 154, 255); // dot
    inline constexpr ImU32 kStatusIdle     = IM_COL32( 90,  92, 104, 255);
    inline constexpr ImU32 kStatusText     = IM_COL32(152, 152, 162, 255);

    inline constexpr ImU32 kConsoleBg      = IM_COL32( 20,  22,  28, 255);
    inline constexpr ImU32 kConsoleText    = IM_COL32(122, 122, 132, 255);

    inline constexpr ImU32 kButton         = IM_COL32( 27,  29,  37, 255);
    inline constexpr ImU32 kButtonHovered  = IM_COL32( 33,  35,  45, 255);
    inline constexpr ImU32 kButtonActive   = IM_COL32( 40,  43,  55, 255);
    inline constexpr ImU32 kButtonText     = IM_COL32(158, 158, 168, 255);
    inline constexpr ImU32 kRunningButton  = IM_COL32( 77,  82, 113, 255); // first button while running
    inline constexpr ImU32 kRunningText    = IM_COL32( 36,  38,  51, 255);
}
