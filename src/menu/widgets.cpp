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

    // Page content --------------------------------------------------------------------

    static void DrawLabel(ImDrawList* dl, const ImVec2& pos, ImU32 col, const char* text)
    {
        dl->AddText(ImGui::GetFont(), ImGui::GetFontSize(), pos, col, text, ImGui::FindRenderedTextEnd(text));
    }

    static float LabelWidth(const char* text)
    {
        return ImGui::CalcTextSize(text, nullptr, true).x;
    }

    // Registers a full-width item of the given height. Returns false when clipped.
    static bool AddRow(const char* label, float height, ImGuiID* id, ImRect* bb, float width = -1.0f)
    {
        ImGuiWindow* window = ImGui::GetCurrentWindow();
        if (window->SkipItems)
            return false;
        *id = window->GetID(label);
        const ImVec2 pos = window->DC.CursorPos;
        *bb = ImRect(pos, pos + ImVec2(width > 0.0f ? width : ImGui::GetContentRegionAvail().x, height));
        ImGui::ItemSize(*bb);
        return ImGui::ItemAdd(*bb, *id);
    }

    static void PushPopupStyle(float alpha, const ImVec2& padding, const ImVec2& spacing)
    {
        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, alpha);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, padding);
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, spacing);
        ImGui::PushStyleVar(ImGuiStyleVar_PopupRounding, style::kWidgetRounding);
        ImGui::PushStyleVar(ImGuiStyleVar_PopupBorderSize, 1.0f);
        ImGui::PushStyleColor(ImGuiCol_PopupBg, style::kPanelBg);
        ImGui::PushStyleColor(ImGuiCol_Border, style::kFrameBg);
    }

    static void PopPopupStyle()
    {
        ImGui::PopStyleColor(2);
        ImGui::PopStyleVar(5);
    }

    // Row inside a Combo/MultiCombo popup.
    static bool PopupItem(const char* text, bool selected, bool check_mark)
    {
        ImGuiWindow* window = ImGui::GetCurrentWindow();
        const ImVec2 pos = window->DC.CursorPos;
        const ImVec2 size(ImGui::GetContentRegionAvail().x, 24.0f);
        const bool pressed = ImGui::InvisibleButton(text, size);
        const float hover_t = Animate(ImGui::GetItemID() + 1, ImGui::IsItemHovered() ? 1.0f : 0.0f, 20.0f);

        ImDrawList* dl = window->DrawList;
        if (hover_t > 0.0f)
            dl->AddRectFilled(pos, pos + size, MulAlpha(style::kFrameBg, hover_t), 3.0f);
        if (selected)
            dl->AddRectFilled(ImVec2(pos.x, pos.y + 6.0f), ImVec2(pos.x + 2.0f, pos.y + size.y - 6.0f), style::kAccent, 1.0f);
        DrawLabel(dl, ImVec2(pos.x + 10.0f, pos.y + IM_TRUNC((size.y - ImGui::GetFontSize()) * 0.5f)), selected ? style::kText : LerpColor(style::kLabel, style::kText, hover_t), text);
        if (check_mark && selected)
            ImGui::RenderCheckMark(dl, ImVec2(pos.x + size.x - 20.0f, pos.y + 7.0f), style::kAccentHovered, 10.0f);
        return pressed;
    }

    static void DrawChevron(ImDrawList* dl, const ImVec2& center, float up_t, ImU32 col)
    {
        const float s = 4.0f;
        const float dir = 1.0f - 2.0f * up_t; // 1 = pointing down, -1 = pointing up
        const ImVec2 pts[3] = {
            ImVec2(center.x - s, center.y - s * 0.5f * dir),
            ImVec2(center.x,     center.y + s * 0.5f * dir),
            ImVec2(center.x + s, center.y - s * 0.5f * dir),
        };
        dl->AddPolyline(pts, 3, col, ImDrawFlags_None, 1.5f);
    }

    void BeginPanel(const char* title, const ImVec2& size_arg)
    {
        ImGuiWindow* window = ImGui::GetCurrentWindow();
        const ImVec2 avail = ImGui::GetContentRegionAvail();
        const ImVec2 size(size_arg.x > 0.0f ? size_arg.x : ImMax(1.0f, avail.x + size_arg.x),
                          size_arg.y > 0.0f ? size_arg.y : ImMax(1.0f, avail.y + size_arg.y));

        ImGui::BeginGroup();
        const ImVec2 pos = window->DC.CursorPos;
        ImDrawList* dl = window->DrawList;
        dl->AddRectFilled(pos, pos + size, style::kPanelBg, style::kWidgetRounding);
        dl->AddText(fonts::tab, style::kFontSize, ImVec2(pos.x + style::kPanelPadding.x, pos.y + IM_TRUNC((style::kPanelHeaderHeight - style::kFontSize) * 0.5f)), style::kText, title, ImGui::FindRenderedTextEnd(title));
        dl->AddRectFilled(ImVec2(pos.x + style::kPanelPadding.x, pos.y + style::kPanelHeaderHeight - 1.0f), ImVec2(pos.x + size.x - style::kPanelPadding.x, pos.y + style::kPanelHeaderHeight), style::kFrameBg);

        ImGui::SetCursorScreenPos(ImVec2(pos.x, pos.y + style::kPanelHeaderHeight));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, style::kPanelPadding);
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(style::kWidgetSpacing, style::kWidgetSpacing));
        ImGui::BeginChild(title, ImVec2(size.x, size.y - style::kPanelHeaderHeight), ImGuiChildFlags_AlwaysUseWindowPadding, ImGuiWindowFlags_NoBackground);
    }

    void EndPanel()
    {
        ImGui::EndChild();
        ImGui::PopStyleVar(2);
        ImGui::EndGroup();
    }

    bool Checkbox(const char* label, bool* v)
    {
        const float h = ImGui::GetFrameHeight();
        const float box = style::kCheckboxSize;
        ImGuiID id;
        ImRect bb;
        if (!AddRow(label, h, &id, &bb, box + 10.0f + LabelWidth(label)))
            return false;

        bool hovered, held;
        const bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held);
        if (pressed)
        {
            *v = !*v;
            ImGui::MarkItemEdited(id);
        }

        const float check_t = Animate(id, *v ? 1.0f : 0.0f);
        const float hover_t = Animate(id + 1, hovered ? 1.0f : 0.0f);

        ImDrawList* dl = ImGui::GetWindowDrawList();
        const ImVec2 box_min(bb.Min.x, bb.Min.y + IM_TRUNC((h - box) * 0.5f));
        const ImVec2 box_max = box_min + ImVec2(box, box);
        dl->AddRectFilled(box_min, box_max, LerpColor(LerpColor(style::kFrameBg, style::kFrameBgHovered, hover_t), LerpColor(style::kAccent, style::kAccentHovered, hover_t), check_t), style::kWidgetRounding);
        if (check_t > 0.0f)
            ImGui::RenderCheckMark(dl, box_min + ImVec2(4.0f, 4.0f), MulAlpha(style::kText, check_t), box - 8.0f);

        DrawLabel(dl, ImVec2(box_max.x + 10.0f, bb.Min.y + ImGui::GetStyle().FramePadding.y), LerpColor(style::kLabel, style::kText, ImMax(check_t, hover_t)), label);
        return pressed;
    }

    bool Toggle(const char* label, bool* v)
    {
        const float h = ImGui::GetFrameHeight();
        ImGuiID id;
        ImRect bb;
        if (!AddRow(label, h, &id, &bb))
            return false;

        bool hovered, held;
        const bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held);
        if (pressed)
        {
            *v = !*v;
            ImGui::MarkItemEdited(id);
        }

        const float on_t = Animate(id, *v ? 1.0f : 0.0f);
        const float hover_t = Animate(id + 1, hovered ? 1.0f : 0.0f);

        ImDrawList* dl = ImGui::GetWindowDrawList();
        const ImVec2 sw = style::kToggleSize;
        const ImVec2 sw_min(bb.Max.x - sw.x, bb.Min.y + IM_TRUNC((h - sw.y) * 0.5f));
        const float radius = sw.y * 0.5f;
        dl->AddRectFilled(sw_min, sw_min + sw, LerpColor(LerpColor(style::kFrameBg, style::kFrameBgHovered, hover_t), LerpColor(style::kAccent, style::kAccentHovered, hover_t), on_t), radius);
        const float knob_x = ImLerp(sw_min.x + radius, sw_min.x + sw.x - radius, on_t);
        dl->AddCircleFilled(ImVec2(knob_x, sw_min.y + radius), radius - 3.0f, LerpColor(style::kKnobOff, style::kKnob, on_t));

        DrawLabel(dl, ImVec2(bb.Min.x, bb.Min.y + ImGui::GetStyle().FramePadding.y), LerpColor(style::kLabel, style::kText, ImMax(on_t, hover_t)), label);
        return pressed;
    }

    static bool SliderImpl(const char* label, float* v_float, int* v_int, float v_min, float v_max, const char* format)
    {
        const float font_size = ImGui::GetFontSize();
        const float track_y = font_size + 7.0f;
        const float height = track_y + style::kSliderTrackHeight + 4.0f;
        ImGuiID id;
        ImRect bb;
        if (!AddRow(label, height, &id, &bb))
            return false;

        bool hovered, held;
        ImGui::ButtonBehavior(bb, id, &hovered, &held);

        const float knob_r = style::kSliderKnobRadius;
        const float track_min = bb.Min.x + knob_r;
        const float track_max = bb.Max.x - knob_r;
        bool changed = false;
        if (held && track_max > track_min)
        {
            const float t = ImSaturate((ImGui::GetIO().MousePos.x - track_min) / (track_max - track_min));
            if (v_float)
            {
                const float nv = ImLerp(v_min, v_max, t);
                changed = nv != *v_float;
                *v_float = nv;
            }
            else
            {
                const int nv = (int)IM_ROUND(ImLerp(v_min, v_max, t));
                changed = nv != *v_int;
                *v_int = nv;
            }
            if (changed)
                ImGui::MarkItemEdited(id);
        }

        const float value = v_float ? *v_float : (float)*v_int;
        const float frac = v_max > v_min ? ImSaturate((value - v_min) / (v_max - v_min)) : 0.0f;
        const float frac_t = Animate(id, frac, 20.0f);
        const float hover_t = Animate(id + 1, (hovered || held) ? 1.0f : 0.0f);

        char buf[64];
        if (v_float)
            ImFormatString(buf, IM_ARRAYSIZE(buf), format, *v_float);
        else
            ImFormatString(buf, IM_ARRAYSIZE(buf), format, *v_int);

        ImDrawList* dl = ImGui::GetWindowDrawList();
        DrawLabel(dl, bb.Min, LerpColor(style::kLabel, style::kText, hover_t), label);
        const float value_w = ImGui::CalcTextSize(buf).x;
        dl->AddText(ImVec2(bb.Max.x - value_w, bb.Min.y), style::kText, buf);

        const float ty = bb.Min.y + track_y;
        const float fill_x = ImLerp(track_min, track_max, frac_t);
        dl->AddRectFilled(ImVec2(bb.Min.x, ty), ImVec2(bb.Max.x, ty + style::kSliderTrackHeight), style::kFrameBg, style::kSliderTrackHeight * 0.5f);
        dl->AddRectFilled(ImVec2(bb.Min.x, ty), ImVec2(fill_x, ty + style::kSliderTrackHeight), LerpColor(style::kAccent, style::kAccentHovered, hover_t), style::kSliderTrackHeight * 0.5f);
        dl->AddCircleFilled(ImVec2(fill_x, ty + style::kSliderTrackHeight * 0.5f), knob_r + hover_t, style::kKnob);
        return changed;
    }

    bool SliderFloat(const char* label, float* v, float v_min, float v_max, const char* format)
    {
        return SliderImpl(label, v, nullptr, v_min, v_max, format);
    }

    bool SliderInt(const char* label, int* v, int v_min, int v_max, const char* format)
    {
        return SliderImpl(label, nullptr, v, (float)v_min, (float)v_max, format);
    }

    // Places the next popup under `anchor`, or above it when there is no room below. The popup height of the
    // previous frame is remembered in the parent window's storage under `key`.
    static void PlacePopup(ImGuiID key, const ImRect& anchor, bool align_right)
    {
        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        const float height = ImGui::GetStateStorage()->GetFloat(key, 0.0f);
        const float gap = 4.0f;
        const bool above = height > 0.0f
            && anchor.Max.y + gap + height > viewport->WorkPos.y + viewport->WorkSize.y
            && anchor.Min.y - gap - height >= viewport->WorkPos.y;
        const ImVec2 pos(align_right ? anchor.Max.x : anchor.Min.x, above ? anchor.Min.y - gap : anchor.Max.y + gap);
        ImGui::SetNextWindowPos(pos, ImGuiCond_Always, ImVec2(align_right ? 1.0f : 0.0f, above ? 1.0f : 0.0f));
    }

    // Shared by Combo and MultiCombo: label, frame with preview text and chevron, and the popup.
    // `items()` draws the popup rows and returns true when the value changed.
    template <typename Items>
    static bool ComboImpl(const char* label, const char* preview, Items&& items)
    {
        const float font_size = ImGui::GetFontSize();
        const float frame_y = font_size + 5.0f;
        ImGuiID id;
        ImRect bb;
        if (!AddRow(label, frame_y + style::kComboHeight, &id, &bb))
            return false;

        ImGui::PushID(label);
        const ImRect frame(ImVec2(bb.Min.x, bb.Min.y + frame_y), bb.Max);
        bool hovered, held;
        if (ImGui::ButtonBehavior(frame, id, &hovered, &held))
            ImGui::OpenPopup("##popup");
        const bool open = ImGui::IsPopupOpen("##popup");

        const float hover_t = Animate(id + 1, (hovered || open) ? 1.0f : 0.0f);
        const float open_t = Animate(id + 2, open ? 1.0f : 0.0f, 18.0f);

        ImDrawList* dl = ImGui::GetWindowDrawList();
        DrawLabel(dl, bb.Min, LerpColor(style::kLabel, style::kText, hover_t), label);
        dl->AddRectFilled(frame.Min, frame.Max, LerpColor(style::kFrameBg, style::kFrameBgHovered, hover_t), style::kWidgetRounding);

        const float text_y = frame.Min.y + IM_TRUNC((style::kComboHeight - font_size) * 0.5f);
        dl->PushClipRect(frame.Min, ImVec2(frame.Max.x - 26.0f, frame.Max.y), true);
        dl->AddText(ImVec2(frame.Min.x + 10.0f, text_y), style::kText, preview);
        dl->PopClipRect();
        DrawChevron(dl, ImVec2(frame.Max.x - 14.0f, frame.GetCenter().y), open_t, LerpColor(style::kLabel, style::kText, hover_t));

        bool changed = false;
        ImGuiStorage* storage = ImGui::GetStateStorage();
        PlacePopup(id + 3, frame, false);
        ImGui::SetNextWindowSize(ImVec2(frame.GetWidth(), 0.0f));
        PushPopupStyle(ImMax(open_t, 0.01f), ImVec2(4.0f, 4.0f), ImVec2(0.0f, 2.0f));
        if (ImGui::BeginPopup("##popup", ImGuiWindowFlags_NoMove))
        {
            changed = items();
            storage->SetFloat(id + 3, ImGui::GetWindowHeight());
            ImGui::EndPopup();
        }
        PopPopupStyle();
        ImGui::PopID();
        if (changed)
            ImGui::MarkItemEdited(id);
        return changed;
    }

    bool Combo(const char* label, int* current, const char* const items[], int items_count)
    {
        const char* preview = (*current >= 0 && *current < items_count) ? items[*current] : "";
        return ComboImpl(label, preview, [&] {
            bool changed = false;
            for (int i = 0; i < items_count; i++)
            {
                ImGui::PushID(i);
                if (PopupItem(items[i], i == *current, false))
                {
                    changed = *current != i;
                    *current = i;
                    ImGui::CloseCurrentPopup();
                }
                ImGui::PopID();
            }
            return changed;
        });
    }

    bool MultiCombo(const char* label, bool* selected, const char* const items[], int items_count)
    {
        char preview[256] = "None";
        int written = 0;
        for (int i = 0; i < items_count; i++)
        {
            if (selected[i])
                written += ImFormatString(preview + written, IM_ARRAYSIZE(preview) - written, written ? ", %s" : "%s", items[i]);
        }

        return ComboImpl(label, preview, [&] {
            bool changed = false;
            for (int i = 0; i < items_count; i++)
            {
                ImGui::PushID(i);
                if (PopupItem(items[i], selected[i], true))
                {
                    selected[i] = !selected[i];
                    changed = true;
                }
                ImGui::PopID();
            }
            return changed;
        });
    }

    bool ColorEdit(const char* label, float col[4], bool alpha)
    {
        const float h = ImGui::GetFrameHeight();
        ImGuiID id;
        ImRect bb;
        if (!AddRow(label, h, &id, &bb))
            return false;

        ImGui::PushID(label);
        bool hovered, held;
        if (ImGui::ButtonBehavior(bb, id, &hovered, &held))
            ImGui::OpenPopup("##picker");
        const bool open = ImGui::IsPopupOpen("##picker");
        const float hover_t = Animate(id + 1, (hovered || open) ? 1.0f : 0.0f);
        const float open_t = Animate(id + 2, open ? 1.0f : 0.0f, 18.0f);

        ImDrawList* dl = ImGui::GetWindowDrawList();
        DrawLabel(dl, ImVec2(bb.Min.x, bb.Min.y + ImGui::GetStyle().FramePadding.y), LerpColor(style::kLabel, style::kText, hover_t), label);

        const ImVec2 sw = style::kColorSwatchSize;
        const ImVec2 sw_min(bb.Max.x - sw.x, bb.Min.y + IM_TRUNC((h - sw.y) * 0.5f));
        const ImU32 color = ImGui::ColorConvertFloat4ToU32(ImVec4(col[0], col[1], col[2], alpha ? col[3] : 1.0f));
        ImGui::RenderColorRectWithAlphaCheckerboard(dl, sw_min, sw_min + sw, color, 5.0f, ImVec2(0.0f, 0.0f), 3.0f);
        dl->AddRect(sw_min, sw_min + sw, LerpColor(style::kFrameBg, style::kFrameBgHovered, hover_t), 3.0f);

        bool changed = false;
        ImGuiStorage* storage = ImGui::GetStateStorage();
        PlacePopup(id + 3, ImRect(sw_min, sw_min + sw), true);
        PushPopupStyle(ImMax(open_t, 0.01f), ImVec2(8.0f, 8.0f), ImVec2(4.0f, 4.0f));
        if (ImGui::BeginPopup("##picker", ImGuiWindowFlags_NoMove))
        {
            ImGuiColorEditFlags flags = ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoSmallPreview | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_PickerHueBar | ImGuiColorEditFlags_DisplayHex;
            flags |= alpha ? (ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_AlphaPreviewHalf) : ImGuiColorEditFlags_NoAlpha;
            ImGui::SetNextItemWidth(190.0f);
            changed = ImGui::ColorPicker4("##color", col, flags);
            storage->SetFloat(id + 3, ImGui::GetWindowHeight());
            ImGui::EndPopup();
        }
        PopPopupStyle();
        ImGui::PopID();
        if (changed)
            ImGui::MarkItemEdited(id);
        return changed;
    }

    bool Keybind(const char* label, int* key)
    {
        static ImGuiID waiting_id = 0;
        static int waiting_frame = 0;

        const float h = ImGui::GetFrameHeight();
        ImGuiID id;
        ImRect bb;
        if (!AddRow(label, h, &id, &bb))
            return false;

        const bool waiting = waiting_id == id;
        const char* text = waiting ? "..." : (*key == ImGuiKey_None ? "None" : ImGui::GetKeyName((ImGuiKey)*key));
        const float text_w = ImGui::CalcTextSize(text).x;
        const ImVec2 btn_size(ImMax(text_w + 20.0f, 50.0f), 20.0f);
        const ImRect btn(ImVec2(bb.Max.x - btn_size.x, bb.Min.y + IM_TRUNC((h - btn_size.y) * 0.5f)), ImVec2(bb.Max.x, bb.Min.y + IM_TRUNC((h - btn_size.y) * 0.5f) + btn_size.y));

        bool hovered, held;
        if (ImGui::ButtonBehavior(btn, id, &hovered, &held))
        {
            waiting_id = id;
            waiting_frame = ImGui::GetFrameCount();
        }

        bool changed = false;
        if (waiting && ImGui::GetFrameCount() > waiting_frame)
        {
            if (ImGui::IsKeyPressed(ImGuiKey_Escape, false))
            {
                changed = *key != ImGuiKey_None;
                *key = ImGuiKey_None;
                waiting_id = 0;
            }
            else
            {
                for (int k = ImGuiKey_NamedKey_BEGIN; k < ImGuiKey_NamedKey_END; k++)
                {
                    if (k >= ImGuiKey_ReservedForModCtrl && k <= ImGuiKey_ReservedForModSuper)
                        continue;
                    if (ImGui::IsKeyPressed((ImGuiKey)k, false))
                    {
                        changed = *key != k;
                        *key = k;
                        waiting_id = 0;
                        break;
                    }
                }
            }
            if (changed)
                ImGui::MarkItemEdited(id);
        }

        const float hover_t = Animate(id + 1, (hovered || waiting) ? 1.0f : 0.0f);
        ImDrawList* dl = ImGui::GetWindowDrawList();
        DrawLabel(dl, ImVec2(bb.Min.x, bb.Min.y + ImGui::GetStyle().FramePadding.y), style::kLabel, label);
        dl->AddRectFilled(btn.Min, btn.Max, LerpColor(style::kFrameBg, style::kFrameBgHovered, hover_t), style::kWidgetRounding);
        dl->AddText(ImVec2(IM_ROUND(btn.GetCenter().x - text_w * 0.5f), btn.Min.y + IM_TRUNC((btn_size.y - ImGui::GetFontSize()) * 0.5f)), waiting ? style::kAccentHovered : style::kText, text);
        return changed;
    }

    bool Button(const char* label, float width)
    {
        ImGuiID id;
        ImRect bb;
        if (!AddRow(label, style::kButtonHeight, &id, &bb, width))
            return false;

        bool hovered, held;
        const bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held);
        const float hover_t = Animate(id + 1, hovered ? 1.0f : 0.0f);

        ImDrawList* dl = ImGui::GetWindowDrawList();
        const ImU32 fill = held ? style::kTabGradientBot : LerpColor(style::kAccent, style::kAccentHovered, hover_t);
        dl->AddRectFilled(bb.Min, bb.Max, fill, style::kWidgetRounding);

        ImFont* font = fonts::tab;
        const char* end = ImGui::FindRenderedTextEnd(label);
        const float text_w = font->CalcTextSizeA(style::kFontSize, FLT_MAX, 0.0f, label, end).x;
        dl->AddText(font, style::kFontSize, ImVec2(IM_ROUND(bb.GetCenter().x - text_w * 0.5f), bb.Min.y + IM_TRUNC((style::kButtonHeight - style::kFontSize) * 0.5f)), style::kText, label, end);
        return pressed;
    }
}
