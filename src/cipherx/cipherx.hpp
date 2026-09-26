#pragma once

#include "imgui.h"

namespace cipherx
{
    inline bool open = true;                        // cleared by the X button
    inline bool running = true;                     // status dot/label and the first button ("Running" or "Start")
    inline const char* license_text = "licensed user";

    inline ImVec2 position = ImVec2(0.0f, 0.0f);    // top-left corner of the window (updated when it is dragged)
    inline bool movable = true;                     // drag the window by its top bar; hosts that move their OS
                                                    // window instead set this to false and use IsDragArea()

    // Button callbacks. When null, the click is only written to the console.
    inline void (*on_start)() = nullptr;
    inline void (*on_refresh_session)() = nullptr;
    inline void (*on_inject_private)() = nullptr;

    // Call once after ImGui::CreateContext(): loads the fonts and writes "ready" to the console.
    void Initialize();

    // Call every frame between ImGui::NewFrame() and ImGui::Render().
    void Render();

    // Console. Every line is shown as "> text". Log() can be called from any thread.
    void Log(const char* fmt, ...) IM_FMTARGS(1);
    void ClearLog();

    // True when (x, y), relative to the window, is on the top bar but not on the close button.
    bool IsDragArea(float x, float y);
}
