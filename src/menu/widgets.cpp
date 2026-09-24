#define IMGUI_DEFINE_MATH_OPERATORS
#include "widgets.hpp"

#include "fonts.hpp"
#include "style.hpp"

#include "imgui_internal.h"

#include <cmath>

namespace menu::widgets
{
    float Animate(ImGuiID id, float target, float speed)
    {
        // First use starts at the target, so the menu never animates in on the first frame.
        float* value = ImGui::GetStateStorage()->GetFloatRef(id, target);
        *value = ImLerp(*value, target, ImSaturate(ImGui::GetIO().DeltaTime * speed));
        if (std::fabs(*value - target) < 0.002f)
            *value = target;
        return *value;
    }

    ImU32 LerpColor(ImU32 a, ImU32 b, float t)
    {
        if (t <= 0.0f) return a;
        if (t >= 1.0f) return b;
        return ImGui::ColorConvertFloat4ToU32(ImLerp(ImGui::ColorConvertU32ToFloat4(a), ImGui::ColorConvertU32ToFloat4(b), t));
    }

    static ImU32 MulAlpha(ImU32 col, float alpha)
    {
        const ImU32 a = (ImU32)(((col >> IM_COL32_A_SHIFT) & 0xFF) * ImSaturate(alpha) + 0.5f);
        return (col & ~IM_COL32_A_MASK) | (a << IM_COL32_A_SHIFT);
    }

    bool SidebarButton(const char* id, const char* icon, const char* label, bool selected, const ImVec2& size, float label_alpha)
    {
        ImGuiWindow* window = ImGui::GetCurrentWindow();
        const ImVec2 pos = window->DC.CursorPos;
        const bool pressed = ImGui::InvisibleButton(id, size);
        const bool hovered = ImGui::IsItemHovered();
        const ImGuiID item_id = ImGui::GetItemID();

        const float active_t = Animate(item_id, selected ? 1.0f : 0.0f);
        const float hover_t = Animate(item_id + 1, hovered ? 1.0f : 0.0f);

        ImDrawList* dl = window->DrawList;
        if (active_t > 0.0f)
            dl->AddRectFilled(pos, pos + size, LerpColor(style::kSidebarBg, style::kSidebarActiveBg, active_t));

        const ImU32 color = LerpColor(LerpColor(style::kIcon, style::kIconHovered, hover_t), style::kIconActive, active_t);
        const float line_y = pos.y + IM_TRUNC(size.y * 0.5f) - style::kSidebarIconOffsetY;

        ImFont* icon_font = fonts::icons;
        const float icon_w = icon_font->CalcTextSizeA(style::kIconFontSize, FLT_MAX, 0.0f, icon).x;
        dl->AddText(icon_font, style::kIconFontSize, ImVec2(IM_ROUND(pos.x + (style::kSidebarWidth - icon_w) * 0.5f), line_y), color, icon);

        if (label && label_alpha > 0.0f)
        {
            // Label is vertically centered on the icon, not on the cell.
            const float icon_center = line_y + style::kIconFontSize * 0.5f;
            dl->PushClipRect(pos, pos + size, true);
            dl->AddText(fonts::tab, style::kFontSize, ImVec2(pos.x + style::kSidebarLabelX, IM_TRUNC(icon_center - style::kFontSize * 0.5f)), MulAlpha(color, label_alpha), label);
            dl->PopClipRect();
        }
        return pressed;
    }

    bool Tab(const char* label, bool selected)
    {
        ImGuiWindow* window = ImGui::GetCurrentWindow();
        const ImVec2 pos = window->DC.CursorPos;
        const ImVec2 size(style::kTabWidth, style::kTabHeight);
        const bool pressed = ImGui::InvisibleButton(label, size);
        const bool hovered = ImGui::IsItemHovered();
        const ImGuiID item_id = ImGui::GetItemID();

        const float active_t = Animate(item_id, selected ? 1.0f : 0.0f);
        const float hover_t = Animate(item_id + 1, hovered ? 1.0f : 0.0f);

        ImDrawList* dl = window->DrawList;
        if (active_t > 0.0f)
        {
            const float line_y = pos.y + size.y - style::kTabLineHeight;
            const ImU32 top = MulAlpha(style::kTabGradientTop, active_t);
            const ImU32 bot = MulAlpha(style::kTabGradientBot, active_t);
            dl->AddRectFilledMultiColor(pos, ImVec2(pos.x + size.x, line_y), top, top, bot, bot);
            dl->AddRectFilled(ImVec2(pos.x, line_y), pos + size, MulAlpha(style::kAccent, active_t), style::kTabLineRounding);
        }

        ImFont* font = fonts::tab;
        const ImVec2 text_size = font->CalcTextSizeA(style::kFontSize, FLT_MAX, 0.0f, label, ImGui::FindRenderedTextEnd(label));
        const ImVec2 text_pos(IM_ROUND(pos.x + (size.x - text_size.x) * 0.5f), pos.y + style::kTabTextOffsetY);
        const ImU32 color = LerpColor(style::kTabText, style::kTabTextActive, ImMax(active_t, hover_t));
        dl->AddText(font, style::kFontSize, text_pos, color, label, ImGui::FindRenderedTextEnd(label));
        return pressed;
    }

    bool Checkbox(const char* label, bool* v)
    {
        ImGuiWindow* window = ImGui::GetCurrentWindow();
        if (window->SkipItems)
            return false;

        const ImGuiStyle& st = ImGui::GetStyle();
        const ImGuiID id = window->GetID(label);
        const ImVec2 label_size = ImGui::CalcTextSize(label, nullptr, true);
        const float square = ImGui::GetFrameHeight();
        const ImVec2 pos = window->DC.CursorPos;
        const ImRect total_bb(pos, pos + ImVec2(square + (label_size.x > 0.0f ? st.ItemInnerSpacing.x + label_size.x : 0.0f), label_size.y + st.FramePadding.y * 2.0f));
        ImGui::ItemSize(total_bb, st.FramePadding.y);
        if (!ImGui::ItemAdd(total_bb, id))
            return false;

        bool hovered, held;
        const bool pressed = ImGui::ButtonBehavior(total_bb, id, &hovered, &held);
        if (pressed)
        {
            *v = !*v;
            ImGui::MarkItemEdited(id);
        }

        const float check_t = Animate(id, *v ? 1.0f : 0.0f);
        const float hover_t = Animate(id + 1, hovered ? 1.0f : 0.0f);

        ImDrawList* dl = window->DrawList;
        const ImRect box(pos, pos + ImVec2(square, square));
        const float rounding = 3.0f;
        if (hover_t > 0.0f && check_t < 1.0f)
            dl->AddRectFilled(box.Min, box.Max, MulAlpha(style::kSidebarActiveBg, hover_t * (1.0f - check_t)), rounding);
        if (check_t > 0.0f)
        {
            dl->AddRectFilled(box.Min, box.Max, MulAlpha(style::kAccent, check_t), rounding);
            const float pad = ImMax(1.0f, IM_TRUNC(square / 5.0f));
            ImGui::RenderCheckMark(dl, box.Min + ImVec2(pad, pad), MulAlpha(style::kText, check_t), square - pad * 2.0f);
        }

        if (label_size.x > 0.0f)
            ImGui::RenderText(ImVec2(box.Max.x + st.ItemInnerSpacing.x, pos.y + st.FramePadding.y), label);

        return pressed;
    }
}
