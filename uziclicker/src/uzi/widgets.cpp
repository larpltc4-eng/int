#define IMGUI_DEFINE_MATH_OPERATORS
#include "widgets.hpp"

#include "fonts.hpp"
#include "style.hpp"

#include <cstdio>
#include <cstdlib>

namespace uzi::widgets
{
    namespace
    {
        float g_scale = 1.0f;

        ImU32 WithAlpha(ImU32 col, float alpha)
        {
            const ImU32 a = (ImU32)(((col >> IM_COL32_A_SHIFT) & 0xFF) * ImSaturate(alpha) + 0.5f);
            return (col & ~IM_COL32_A_MASK) | (a << IM_COL32_A_SHIFT);
        }

        ImU32 Mix(ImU32 a, ImU32 b, float t)
        {
            ImU32 out = 0;
            for (int shift = 0; shift < 32; shift += 8)
            {
                const float ca = (float)((a >> shift) & 0xFF), cb = (float)((b >> shift) & 0xFF);
                out |= (ImU32)(ca + (cb - ca) * t + 0.5f) << shift;
            }
            return out;
        }

        // 0..1 value per item that eases towards `on` (hover fades).
        float Animate(ImGuiID id, bool on, float speed = 12.0f)
        {
            float* v = ImGui::GetStateStorage()->GetFloatRef(ImHashStr("##anim", 0, id), on ? 1.0f : 0.0f);
            const float step = ImGui::GetIO().DeltaTime * speed;
            *v = on ? ImMin(*v + step, 1.0f) : ImMax(*v - step, 0.0f);
            return *v;
        }

        // Rounded rectangle path. Unlike ImDrawList::PathRect the radius is only limited to half
        // of the smaller side, so pills get exact semicircular ends.
        void PathRound(ImDrawList* dl, const ImVec2& a, const ImVec2& b, float r)
        {
            r = ImMin(r, ImMin(b.x - a.x, b.y - a.y) * 0.5f - 0.01f);
            if (r < 0.5f)
            {
                dl->PathLineTo(a);
                dl->PathLineTo(ImVec2(b.x, a.y));
                dl->PathLineTo(b);
                dl->PathLineTo(ImVec2(a.x, b.y));
                return;
            }
            dl->PathArcTo(ImVec2(a.x + r, a.y + r), r, IM_PI, IM_PI * 1.5f);
            dl->PathArcTo(ImVec2(b.x - r, a.y + r), r, IM_PI * 1.5f, IM_PI * 2.0f);
            dl->PathArcTo(ImVec2(b.x - r, b.y - r), r, 0.0f, IM_PI * 0.5f);
            dl->PathArcTo(ImVec2(a.x + r, b.y - r), r, IM_PI * 0.5f, IM_PI);
        }

        void FillRound(ImDrawList* dl, const ImRect& bb, float r, ImU32 col)
        {
            PathRound(dl, bb.Min, bb.Max, r);
            dl->PathFillConvex(col);
        }

        // Outline whose outer edge lies exactly on bb (ImDrawList::AddRect centers it on bb shrunk by 0.5px).
        void StrokeRound(ImDrawList* dl, const ImRect& bb, float r, float thickness, ImU32 col)
        {
            const float h = thickness * 0.5f;
            PathRound(dl, bb.Min + ImVec2(h, h), bb.Max - ImVec2(h, h), r - h);
            dl->PathStroke(col, ImDrawFlags_Closed, thickness);
        }

        void RadialGradient(ImDrawList* dl, const ImVec2& center, float radius, ImU32 col_center, ImU32 col_edge)
        {
            const int segments = 32;
            const ImVec2 uv = dl->_Data->TexUvWhitePixel;
            dl->PrimReserve(segments * 3, segments + 1);
            const ImDrawIdx base = (ImDrawIdx)dl->_VtxCurrentIdx;
            dl->PrimWriteVtx(center, uv, col_center);
            for (int i = 0; i < segments; i++)
            {
                const float a = (float)i / (float)segments * IM_PI * 2.0f;
                dl->PrimWriteVtx(ImVec2(center.x + ImCos(a) * radius, center.y + ImSin(a) * radius), uv, col_edge);
            }
            for (int i = 0; i < segments; i++)
            {
                dl->PrimWriteIdx(base);
                dl->PrimWriteIdx((ImDrawIdx)(base + 1 + i));
                dl->PrimWriteIdx((ImDrawIdx)(base + 1 + (i + 1) % segments));
            }
        }

        // Dear ImGui 1.92 moved the per-size font data into ImFontBaked, these work with 1.91 and 1.92+.
        float FontSize(ImFont* font)
        {
#if IMGUI_VERSION_NUM >= 19200
            return font->LegacySize;
#else
            return font->FontSize;
#endif
        }

        float FontAscent(ImFont* font)
        {
#if IMGUI_VERSION_NUM >= 19200
            return font->GetFontBaked(font->LegacySize)->Ascent;
#else
            return font->Ascent;
#endif
        }

        // `pos` is in screen coordinates: x and the baseline.
        void TextAtBaseline(ImFont* font, const ImVec2& pos, ImU32 col, const char* text)
        {
            ImGui::GetWindowDrawList()->AddText(font, FontSize(font), ImVec2(pos.x, pos.y - FontAscent(font)), col, text);
        }

        void TextCentered(ImFont* font, const ImRect& bb, float baseline, ImU32 col, const char* text)
        {
            TextAtBaseline(font, ImVec2(bb.GetCenter().x - TextWidth(font, text) * 0.5f, bb.Min.y + Px(baseline)), col, text);
        }

        int DigitsOnly(ImGuiInputTextCallbackData* data)
        {
            return (data->EventChar >= '0' && data->EventChar <= '9') ? 0 : 1;
        }

        // Text buffers of the value boxes while they are being edited.
        struct EditBuffer
        {
            ImGuiID id;
            char    text[12];
        };
        ImVector<EditBuffer> g_buffers;

        char* BufferFor(ImGuiID id)
        {
            for (EditBuffer& b : g_buffers)
                if (b.id == id)
                    return b.text;
            g_buffers.push_back(EditBuffer{ id, {} });
            return g_buffers.back().text;
        }
    }

    void  SetScale(float scale) { g_scale = scale; }
    float GetScale() { return g_scale; }

    float Px(float v) { return IM_ROUND(v * g_scale); }

    ImVec2 Pos(float x, float y)
    {
        const ImVec2 origin = ImGui::GetCurrentWindow()->Pos;
        return ImVec2(origin.x + Px(x), origin.y + Px(y));
    }

    ImRect Rect(float x, float y, float w, float h)
    {
        const ImVec2 origin = ImGui::GetCurrentWindow()->Pos;
        return ImRect(origin.x + Px(x), origin.y + Px(y), origin.x + Px(x + w), origin.y + Px(y + h));
    }

    float TextWidth(ImFont* font, const char* text)
    {
        return font->CalcTextSizeA(FontSize(font), FLT_MAX, 0.0f, text).x;
    }

    void Text(ImFont* font, float x, float baseline, ImU32 color, const char* text)
    {
        TextAtBaseline(font, Pos(x, baseline), color, text);
    }

    void Label(float x, float baseline, const char* text, const char* accent_text)
    {
        ImVec2 pos = Pos(x, baseline);
        TextAtBaseline(fonts::text, pos, style::kText, text);
        pos.x += TextWidth(fonts::text, text);
        TextAtBaseline(fonts::text, pos, style::kAccent, accent_text);
    }

    const char* KeyName(ImGuiKey key)
    {
        switch (key)
        {
        case ImGuiKey_None:        return "?";
        case ImGuiKey_MouseMiddle: return "Mysz 3";
        case ImGuiKey_MouseX1:     return "Mysz 4";
        case ImGuiKey_MouseX2:     return "Mysz 5";
        default:                   return ImGui::GetKeyName(key);
        }
    }

    bool SidebarButton(const char* label, bool selected, const ImRect& bb)
    {
        ImGuiWindow* window = ImGui::GetCurrentWindow();
        const ImGuiID id = window->GetID(label);
        if (!ImGui::ItemAdd(bb, id))
            return false;

        bool hovered, held;
        const bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held);
        const float hover = Animate(id, hovered || held);

        ImDrawList* dl = window->DrawList;
        const float r = bb.GetHeight() * 0.5f;
        if (selected)
        {
            FillRound(dl, bb, r, style::kAccentFill);
        }
        else
        {
            if (hover > 0.0f)
                FillRound(dl, bb, r, WithAlpha(held ? style::kAccentActive : style::kAccentHover, hover));
            StrokeRound(dl, bb, r, style::kBorder * g_scale, style::kAccent);
        }
        TextCentered(fonts::sidebar, bb, style::kSidebarBaseline, style::kText, label);
        return pressed;
    }

    bool BindButton(const char* str_id, ImGuiKey* key, const ImRect& bb)
    {
        static ImGuiID waiting_id = 0;
        static int     waiting_frame = 0;

        ImGuiWindow* window = ImGui::GetCurrentWindow();
        const ImGuiID id = window->GetID(str_id);
        if (!ImGui::ItemAdd(bb, id))
            return false;

        bool hovered, held;
        if (ImGui::ButtonBehavior(bb, id, &hovered, &held))
        {
            waiting_id = (waiting_id == id) ? 0 : id; // a second click cancels
            waiting_frame = ImGui::GetFrameCount();
        }

        bool changed = false;
        if (waiting_id == id && ImGui::GetFrameCount() > waiting_frame)
        {
            if (ImGui::IsKeyPressed(ImGuiKey_Escape, false) || ImGui::IsKeyPressed(ImGuiKey_Backspace, false) || ImGui::IsKeyPressed(ImGuiKey_Delete, false))
            {
                changed = *key != ImGuiKey_None;
                *key = ImGuiKey_None;
                waiting_id = 0;
            }
            else if (!hovered && (ImGui::IsMouseClicked(ImGuiMouseButton_Left) || ImGui::IsMouseClicked(ImGuiMouseButton_Right)))
            {
                waiting_id = 0; // clicked somewhere else
            }
            else
            {
                for (int k = ImGuiKey_NamedKey_BEGIN; k < ImGuiKey_NamedKey_END; k++)
                {
                    if (k == ImGuiKey_MouseLeft || k == ImGuiKey_MouseRight || k == ImGuiKey_MouseWheelX || k == ImGuiKey_MouseWheelY ||
                        (k >= ImGuiKey_ReservedForModCtrl && k <= ImGuiKey_ReservedForModSuper))
                        continue;
                    if (ImGui::IsKeyPressed((ImGuiKey)k, false))
                    {
                        changed = *key != (ImGuiKey)k;
                        *key = (ImGuiKey)k;
                        waiting_id = 0;
                        break;
                    }
                }
            }
            if (changed)
                ImGui::MarkItemEdited(id);
        }

        const bool waiting = waiting_id == id;
        const float hover = Animate(id, hovered || held || waiting);
        ImDrawList* dl = window->DrawList;
        const float r = bb.GetHeight() * 0.5f;
        if (hover > 0.0f)
            FillRound(dl, bb, r, WithAlpha(held ? style::kAccentActive : style::kAccentHover, hover));
        StrokeRound(dl, bb, r, style::kBorder * g_scale, waiting ? style::kSlider : style::kAccent);

        char text[64];
        std::snprintf(text, sizeof(text), "Bind: %s", waiting ? "..." : KeyName(*key));
        TextCentered(fonts::text, bb, style::kBindBaseline, style::kText, text);
        return changed;
    }

    bool Dropdown(const char* str_id, int* current, const char* const items[], int count, const ImRect& bb)
    {
        ImGuiWindow* window = ImGui::GetCurrentWindow();
        const ImGuiID id = window->GetID(str_id);
        if (!ImGui::ItemAdd(bb, id))
            return false;

        const ImGuiID popup_id = ImHashStr("##list", 0, id);
        bool hovered, held;
        const bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held);
        bool open = ImGui::IsPopupOpen(popup_id, ImGuiPopupFlags_None);
        if (pressed && !open)
        {
            ImGui::OpenPopupEx(popup_id, ImGuiPopupFlags_None);
            open = true;
        }

        const float hover = Animate(id, hovered || held || open);
        ImDrawList* dl = window->DrawList;
        FillRound(dl, bb, Px(style::kDropdownRounding), Mix(style::kDropdown, style::kDropdownHover, hover));
        const int index = ImClamp(*current, 0, count - 1);
        TextAtBaseline(fonts::text, ImVec2(bb.Min.x + Px(style::kDropdownTextX), bb.Min.y + Px(style::kDropdownBaseline)), style::kDropdownText, items[index]);

        // Small filled triangle, pointing up while the list is open.
        const float cx = bb.Max.x - style::kArrowRight * g_scale;
        const float top = bb.Min.y + style::kArrowTop * g_scale;
        const float hw = style::kArrowSize.x * 0.5f * g_scale;
        const float h = style::kArrowSize.y * g_scale;
        if (open)
            dl->AddTriangleFilled(ImVec2(cx - hw, top + h), ImVec2(cx, top), ImVec2(cx + hw, top + h), style::kDropdownText);
        else
            dl->AddTriangleFilled(ImVec2(cx - hw, top), ImVec2(cx + hw, top), ImVec2(cx, top + h), style::kDropdownText);

        bool changed = false;
        if (open)
        {
            ImGui::SetNextWindowPos(ImVec2(bb.Min.x, bb.Max.y + Px(4.0f)));
            ImGui::SetNextWindowSize(ImVec2(bb.GetWidth(), 0.0f));
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, Px(4.0f)));
            ImGui::PushStyleVar(ImGuiStyleVar_PopupRounding, Px(style::kDropdownRounding));
            ImGui::PushStyleVar(ImGuiStyleVar_PopupBorderSize, 0.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));
            ImGui::PushStyleColor(ImGuiCol_PopupBg, style::kDropdown);
            const ImGuiWindowFlags flags = ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove |
                                           ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoScrollbar;
            if (ImGui::BeginPopupEx(popup_id, flags))
            {
                for (int i = 0; i < count; i++)
                {
                    ImGui::PushID(i);
                    const bool clicked = ImGui::InvisibleButton("##item", ImVec2(bb.GetWidth(), Px(style::kItemHeight)));
                    const ImRect item(ImGui::GetItemRectMin(), ImGui::GetItemRectMax());
                    if (ImGui::IsItemHovered())
                        ImGui::GetWindowDrawList()->AddRectFilled(item.Min + ImVec2(Px(4.0f), 0.0f), item.Max - ImVec2(Px(4.0f), 0.0f), style::kItemHover, Px(6.0f));
                    TextAtBaseline(fonts::text, ImVec2(item.Min.x + Px(style::kDropdownTextX), item.Min.y + Px(style::kDropdownBaseline)),
                                   i == index ? style::kAccent : style::kDropdownText, items[i]);
                    if (clicked)
                    {
                        changed = *current != i;
                        *current = i;
                        ImGui::CloseCurrentPopup();
                    }
                    ImGui::PopID();
                }
                ImGui::EndPopup();
            }
            ImGui::PopStyleColor();
            ImGui::PopStyleVar(4);
        }
        if (changed)
            ImGui::MarkItemEdited(id);
        return changed;
    }

    bool Slider(const char* str_id, int* v, int v_min, int v_max, float track_center_y)
    {
        ImGuiWindow* window = ImGui::GetCurrentWindow();
        const ImGuiID id = window->GetID(str_id);
        const ImRect track = Rect(style::kTrackX0, track_center_y - style::kTrackHeight * 0.5f, style::kTrackX1 - style::kTrackX0, style::kTrackHeight);
        const float knob_r = style::kKnobRadius * g_scale;
        const float cy = track.GetCenter().y;
        const ImRect hit(track.Min.x - knob_r, cy - Px(9.0f), track.Max.x + knob_r, cy + Px(9.0f));
        if (!ImGui::ItemAdd(hit, id))
            return false;

        bool hovered, held;
        ImGui::ButtonBehavior(hit, id, &hovered, &held, ImGuiButtonFlags_PressedOnClick);

        const ImGuiIO& io = ImGui::GetIO();
        int value = ImClamp(*v, v_min, v_max);
        if (held)
            value = v_min + (int)IM_ROUND(ImSaturate((io.MousePos.x - track.Min.x) / track.GetWidth()) * (float)(v_max - v_min));
        else if (hovered && io.MouseWheel != 0.0f)
            value = ImClamp(value + (io.MouseWheel > 0.0f ? 1 : -1), v_min, v_max);
        const bool changed = value != *v;
        if (changed)
        {
            *v = value;
            ImGui::MarkItemEdited(id);
        }

        const float knob_x = track.Min.x + track.GetWidth() * (float)(value - v_min) / (float)(v_max - v_min);
        ImDrawList* dl = window->DrawList;
        dl->AddRectFilled(track.Min, track.Max, style::kSliderRail);
        dl->AddRectFilled(track.Min, ImVec2(knob_x, track.Max.y), style::kSlider);
        dl->AddCircleFilled(ImVec2(knob_x, cy), knob_r + Animate(id, hovered || held) * g_scale, style::kSlider);
        return changed;
    }

    bool ValueBox(const char* str_id, int* v, int v_min, int v_max, const ImRect& bb, float border, float baseline)
    {
        ImGuiWindow* window = ImGui::GetCurrentWindow();
        ImGui::PushID(str_id);
        const ImGuiID input_id = window->GetID("##input");
        char* buf = BufferFor(input_id);
        int max_digits = 1;
        for (int n = v_max; n >= 10; n /= 10)
            max_digits++;

        const bool editing = ImGui::GetActiveID() == input_id;
        if (!editing)
            std::snprintf(buf, sizeof(EditBuffer::text), "%d", ImClamp(*v, v_min, v_max));

        // Frame first, the InputText draws the (centered) number on top of it.
        const bool hovered = ImGui::IsWindowHovered(ImGuiHoveredFlags_None) && ImGui::IsMouseHoveringRect(bb.Min, bb.Max);
        const float hover = Animate(input_id, hovered || editing);
        ImDrawList* dl = window->DrawList;
        const float r = bb.GetHeight() * 0.5f;
        if (hover > 0.0f)
            FillRound(dl, bb, r, WithAlpha(style::kAccentHover, hover));
        StrokeRound(dl, bb, r, border * g_scale, editing ? style::kSlider : style::kAccent);

        ImFont* font = fonts::text;
        const float pad_x = ImMax(0.0f, IM_TRUNC((bb.GetWidth() - TextWidth(font, buf)) * 0.5f));
        const float pad_y = ImMax(0.0f, Px(baseline) - FontAscent(font));
        ImGui::SetCursorScreenPos(bb.Min);
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(pad_x, pad_y));
        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
        ImGui::PushStyleColor(ImGuiCol_FrameBg, IM_COL32(0, 0, 0, 0));
        ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, IM_COL32(0, 0, 0, 0));
        ImGui::PushStyleColor(ImGuiCol_FrameBgActive, IM_COL32(0, 0, 0, 0));
        ImGui::SetNextItemWidth(bb.GetWidth());
        ImGui::InputText("##input", buf, (size_t)max_digits + 1, ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_CallbackCharFilter, DigitsOnly);
        const bool commit = ImGui::IsItemDeactivatedAfterEdit();
        ImGui::PopStyleColor(3);
        ImGui::PopStyleVar(2);
        ImGui::PopID();

        if (!commit || buf[0] == 0)
            return false;
        const int value = ImClamp(std::atoi(buf), v_min, v_max);
        const bool changed = value != *v;
        *v = value;
        return changed;
    }

    bool Toggle(const char* str_id, const char* label, bool* v, float label_x, float toggle_x, float baseline)
    {
        ImGuiWindow* window = ImGui::GetCurrentWindow();
        const ImGuiID id = window->GetID(str_id);
        const ImVec2 label_pos = Pos(label_x, baseline);
        const ImVec2 toggle_pos = Pos(toggle_x, baseline);

        // The whole row (label + ON/OFF) is clickable.
        const ImRect bb(ImVec2(label_pos.x - Px(4.0f), label_pos.y - Px(16.0f)),
                        ImVec2(toggle_pos.x + TextWidth(fonts::toggle, "OFF") + Px(4.0f), label_pos.y + Px(6.0f)));
        if (!ImGui::ItemAdd(bb, id))
            return false;

        bool hovered, held;
        const bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held);
        if (pressed)
        {
            *v = !*v;
            ImGui::MarkItemEdited(id);
        }

        const float hover = Animate(id, hovered);
        TextAtBaseline(fonts::text, label_pos, style::kText, label);
        TextAtBaseline(fonts::toggle, toggle_pos, Mix(*v ? style::kOn : style::kOff, IM_COL32_WHITE, hover * 0.3f), *v ? "ON" : "OFF");
        return pressed;
    }

    void Particles(const ImRect& area)
    {
        struct Particle
        {
            ImVec2 pos; // base (100%) coordinates inside `area`
            ImVec2 vel; // px per second
            float  radius;
        };
        static Particle particles[5];
        static bool initialized = false;

        const ImVec2 size = area.GetSize() / g_scale;
        if (!initialized)
        {
            unsigned int seed = 0x2545F491u;
            auto rnd = [&seed]() { seed = seed * 1664525u + 1013904223u; return (float)(seed >> 8) / 16777216.0f; };
            for (Particle& p : particles)
            {
                p.pos = ImVec2(rnd() * size.x, rnd() * size.y);
                const float angle = rnd() * IM_PI * 2.0f;
                const float speed = 6.0f + rnd() * 10.0f;
                p.vel = ImVec2(ImCos(angle) * speed, ImSin(angle) * speed);
                p.radius = 2.5f + rnd() * 3.0f; // the dots on the screenshot are 5-11 px wide
            }
            initialized = true;
        }

        const float dt = ImMin(ImGui::GetIO().DeltaTime, 0.1f);
        ImDrawList* dl = ImGui::GetWindowDrawList();
        for (Particle& p : particles)
        {
            p.pos += p.vel * dt;
            const float margin = p.radius * 3.0f; // wrap around once fully outside
            if (p.pos.x < -margin) p.pos.x += size.x + margin * 2.0f;
            if (p.pos.x > size.x + margin) p.pos.x -= size.x + margin * 2.0f;
            if (p.pos.y < -margin) p.pos.y += size.y + margin * 2.0f;
            if (p.pos.y > size.y + margin) p.pos.y -= size.y + margin * 2.0f;

            const ImVec2 c = area.Min + p.pos * g_scale;
            const float r = p.radius * g_scale;
            RadialGradient(dl, c, r * 2.6f, WithAlpha(style::kParticle, 0.35f), WithAlpha(style::kParticle, 0.0f)); // glow
            RadialGradient(dl, c, r * 1.6f, WithAlpha(style::kParticle, 0.55f), WithAlpha(style::kParticle, 0.0f));
            dl->AddCircleFilled(c, r, style::kParticle);
            RadialGradient(dl, c - ImVec2(r, r) * 0.3f, r * 0.85f, IM_COL32(255, 255, 255, 90), IM_COL32(255, 255, 255, 0)); // shine
        }
    }
}
