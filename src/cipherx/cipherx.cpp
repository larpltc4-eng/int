#define IMGUI_DEFINE_MATH_OPERATORS
#include "cipherx.hpp"

#include "fonts.hpp"
#include "style.hpp"

#include "imgui_internal.h"

#include <cstdarg>
#include <cstring>
#include <mutex>

namespace cipherx
{
    namespace
    {
        constexpr const char* kTitle = "CipherX";

        // Console text: every line is "> message\n"; line_offsets holds where each line starts.
        // Guarded by a mutex because Log() may be called from other threads.
        std::mutex log_mutex;
        ImGuiTextBuffer log_buffer;
        ImVector<int> log_line_offsets;
        bool log_scroll_to_bottom = false;

        ImRect CloseRect(const ImVec2& p)
        {
            const ImVec2 half(style::kCloseHitSize * 0.5f, style::kCloseHitSize * 0.5f);
            return ImRect(p + style::kCloseCenter - half, p + style::kCloseCenter + half);
        }

        void DrawRings(ImDrawList* dl, const ImVec2& p)
        {
            for (int i = 0; i < IM_ARRAYSIZE(style::kRingRadii); i++)
                dl->AddCircle(p + style::kRingCenter, style::kRingRadii[i], style::kRingColors[i], style::kRingSegments, style::kRingThickness);
        }

        void DrawHeader(ImDrawList* dl, const ImVec2& p)
        {
            dl->AddText(fonts::small, style::kSmallFontSize, p + style::kLicensePos, style::kLicense, license_text);

            const float title_w = fonts::title->CalcTextSizeA(style::kTitleFontSize, FLT_MAX, 0.0f, kTitle).x;
            dl->AddText(fonts::title, style::kTitleFontSize, ImVec2(p.x + (style::kWindowSize.x - title_w) * 0.5f, p.y + style::kTitleY), style::kTitle, kTitle);

            // Close button: two 10x10 diagonals.
            const ImRect hit = CloseRect(p);
            ImGui::SetCursorScreenPos(hit.Min);
            if (ImGui::InvisibleButton("##close", hit.GetSize()))
                open = false;
            const ImU32 col = ImGui::IsItemHovered() ? style::kCloseHovered : style::kClose;
            const ImVec2 c = p + style::kCloseCenter;
            const float s = style::kCloseHalfSize;
            dl->AddLine(ImVec2(c.x - s, c.y - s), ImVec2(c.x + s, c.y + s), col, style::kCloseThickness);
            dl->AddLine(ImVec2(c.x + s, c.y - s), ImVec2(c.x - s, c.y + s), col, style::kCloseThickness);

            // Dragging the top bar moves the window (submitted after the close button, which takes priority).
            ImGui::SetCursorScreenPos(p);
            ImGui::InvisibleButton("##drag", ImVec2(style::kWindowSize.x, style::kTopBarHeight));
            if (movable && ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left, 0.0f))
                position += ImGui::GetIO().MouseDelta;
        }

        void DrawStatus(ImDrawList* dl, const ImVec2& p)
        {
            dl->AddCircleFilled(p + style::kStatusDot, style::kStatusDotRadius, running ? style::kStatusRunning : style::kStatusIdle, style::kStatusDotSegments);
            dl->AddText(fonts::small, style::kStatusFontSize, p + style::kStatusTextPos, style::kStatusText, running ? "Running" : "Idle");
        }

        void DrawConsole(const ImVec2& p)
        {
            ImGui::SetCursorScreenPos(p + ImVec2(style::kContentX, style::kConsoleY));
            ImGui::PushStyleColor(ImGuiCol_ChildBg, style::kConsoleBg);
            ImGui::PushStyleColor(ImGuiCol_Border, style::kBorder);
            ImGui::PushStyleColor(ImGuiCol_Text, style::kConsoleText);
            ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, style::kConsoleRounding);
            ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 1.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, style::kConsolePadding);
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, style::kConsoleLineSpacing));
            ImGui::PushFont(fonts::mono);
            if (ImGui::BeginChild("##console", ImVec2(style::kContentWidth, style::kConsoleHeight), ImGuiChildFlags_Borders | ImGuiChildFlags_AlwaysUseWindowPadding, ImGuiWindowFlags_NoScrollbar))
            {
                std::lock_guard<std::mutex> lock(log_mutex);
                ImGuiListClipper clipper;
                clipper.Begin(log_line_offsets.Size);
                while (clipper.Step())
                {
                    for (int line = clipper.DisplayStart; line < clipper.DisplayEnd; line++)
                    {
                        const char* start = log_buffer.begin() + log_line_offsets[line];
                        const char* end = (line + 1 < log_line_offsets.Size) ? log_buffer.begin() + log_line_offsets[line + 1] - 1 : log_buffer.end() - 1;
                        ImGui::TextUnformatted(start, end);
                    }
                }
                clipper.End();
                if (log_scroll_to_bottom)
                    ImGui::SetScrollHereY(1.0f);
                log_scroll_to_bottom = false;
            }
            ImGui::EndChild();
            ImGui::PopFont();
            ImGui::PopStyleVar(4);
            ImGui::PopStyleColor(3);
        }

        bool Button(const char* label, const ImVec2& size)
        {
            ImGui::PushStyleColor(ImGuiCol_Button, style::kButton);
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, style::kButtonHovered);
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, style::kButtonActive);
            ImGui::PushStyleColor(ImGuiCol_Text, style::kButtonText);
            ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
            const bool pressed = ImGui::Button(label, size);
            ImGui::PopStyleVar();
            ImGui::PopStyleColor(4);
            return pressed;
        }

        // The first button shows the status: highlighted "Running" (inactive) while running, "Start" otherwise.
        void RunButton(const ImVec2& size)
        {
            if (!running)
            {
                if (Button("Start", size))
                {
                    running = true;
                    if (on_start)
                        on_start();
                    else
                        Log("start");
                }
                return;
            }
            ImGui::PushStyleColor(ImGuiCol_Button, style::kRunningButton);
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, style::kRunningButton);
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, style::kRunningButton);
            ImGui::PushStyleColor(ImGuiCol_Text, style::kRunningText);
            ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
            ImGui::Button("Running", size);
            ImGui::PopStyleVar();
            ImGui::PopStyleColor(4);
        }

        void DrawButtons(const ImVec2& p)
        {
            const ImVec2 size((style::kContentWidth - style::kButtonSpacing * 2.0f) / 3.0f, style::kButtonHeight);
            ImGui::SetCursorScreenPos(p + ImVec2(style::kContentX, style::kButtonsY));
            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, style::kButtonRounding);
            ImGui::PushStyleColor(ImGuiCol_Border, style::kBorder);
            ImGui::PushStyleColor(ImGuiCol_BorderShadow, IM_COL32_BLACK_TRANS);

            RunButton(size);
            ImGui::SameLine(0.0f, style::kButtonSpacing);
            if (Button("Refresh Session", size))
            {
                if (on_refresh_session)
                    on_refresh_session();
                else
                    Log("refresh session");
            }
            ImGui::SameLine(0.0f, style::kButtonSpacing);
            if (Button("Inject Private", size))
            {
                if (on_inject_private)
                    on_inject_private();
                else
                    Log("inject private");
            }

            ImGui::PopStyleColor(2);
            ImGui::PopStyleVar();
        }
    }

    void Initialize()
    {
        fonts::Load();
        Log("ready");
    }

    void Render()
    {
        if (!open)
            return;

        ImGui::SetNextWindowPos(position, ImGuiCond_Always);
        ImGui::SetNextWindowSize(style::kWindowSize, ImGuiCond_Always);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        const ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoScrollWithMouse;
        const bool visible = ImGui::Begin("##cipherx", nullptr, flags);
        ImGui::PopStyleVar(3);
        if (!visible)
        {
            ImGui::End();
            return;
        }
        ImGui::PushFont(fonts::regular);

        ImDrawList* dl = ImGui::GetWindowDrawList();
        const ImVec2 p = ImGui::GetWindowPos();
        const ImVec2 size = style::kWindowSize;

        dl->AddRectFilled(p, p + size, style::kWindowBg);
        DrawRings(dl, p);
        // AddRect adds another half pixel, so the 1 px line falls between the two outer pixels, as in the reference.
        dl->AddRect(p + ImVec2(0.5f, 0.5f), p + size - ImVec2(0.5f, 0.5f), style::kBorder, style::kWindowRounding);

        DrawHeader(dl, p);
        DrawStatus(dl, p);
        DrawConsole(p);
        DrawButtons(p);

        ImGui::PopFont();
        ImGui::End();
    }

    void Log(const char* fmt, ...)
    {
        char text[1024];
        va_list args;
        va_start(args, fmt);
        ImFormatStringV(text, IM_ARRAYSIZE(text), fmt, args);
        va_end(args);

        std::lock_guard<std::mutex> lock(log_mutex);
        for (const char* line = text; ; )
        {
            const char* line_end = strchr(line, '\n');
            log_line_offsets.push_back(log_buffer.size());
            log_buffer.appendf("> %.*s\n", (int)(line_end ? line_end - line : strlen(line)), line);
            if (!line_end)
                break;
            line = line_end + 1;
        }
        log_scroll_to_bottom = true;
    }

    void ClearLog()
    {
        std::lock_guard<std::mutex> lock(log_mutex);
        log_buffer.clear();
        log_line_offsets.clear();
    }

    bool IsDragArea(float x, float y)
    {
        return x >= 0.0f && x < style::kWindowSize.x && y >= 0.0f && y < style::kTopBarHeight && !CloseRect(ImVec2(0.0f, 0.0f)).Contains(ImVec2(x, y));
    }
}
