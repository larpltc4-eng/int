#define IMGUI_DEFINE_MATH_OPERATORS
#include "tilemenu.hpp"

#include "imgui_internal.h"

#include "fonts/roboto_bold.h"

#include <cstdio>

namespace tilemenu
{
    namespace
    {
        // Every size and color was measured from the reference screenshot (938x841). Offsets are from the window's top-left.
        namespace layout
        {
            constexpr ImVec2 kWindowPos     = { 8.0f, 8.0f };
            constexpr ImVec2 kWindowSize    = { 920.0f, 823.0f }; // including the title bar
            // ImGui sizes a font by its line height (ascent + descent), which is 1.172 em for Roboto:
            // 37.5 px here is the 32 px em measured in the screenshot.
            constexpr float  kFontSize      = 37.5f;
            constexpr ImVec2 kWindowPadding = { 11.0f, 11.0f };
            constexpr ImVec2 kFramePadding  = { 11.0f, 9.25f };   // line height 37 + 2 * 9.25 = 55.5: title bar and tiles
            constexpr ImVec2 kItemSpacing   = { 11.0f, 12.5f };
            constexpr ImVec2 kUnlockPos     = { 11.0f, 211.5f };
            constexpr ImVec2 kUnlockSize    = { 441.0f, 57.5f };
            constexpr float  kUnlockTextNudgeY = 2.0f;           // the label sits a bit below the center of the button

            // Skin page (everything is placed by hand, like in the screenshot): check boxes (41 px square, 7 px frame around the white square), label 13 px to the right.
            constexpr float  kBoxSize       = 41.0f;
            constexpr float  kBoxInset      = 7.0f;
            constexpr float  kBoxLabelGap   = 12.0f;
            constexpr float  kLabelNudgeY   = -1.0f;
            constexpr ImVec2 kCollectionPos = { 21.0f, 291.0f };
            constexpr ImVec2 kEjectPos      = { 21.0f, 354.0f };
            constexpr ImVec2 kBuyBindsPos   = { 476.0f, 354.0f };
            constexpr ImVec2 kBuddyPos      = { 21.0f, 417.0f };

            constexpr ImVec2 kBuddyLabelPos = { 28.0f, 476.0f };  // "buddy" text under the check boxes
            constexpr ImVec2 kSliderPos     = { 28.0f, 535.0f };
            constexpr ImVec2 kSliderSize    = { 880.0f, 18.0f };
            constexpr float  kSliderHitPad  = 8.0f;               // the slider reacts this far above and below the track
            constexpr float  kSliderValueGap = 8.0f;              // track bottom -> value text
            constexpr ImVec2 kBuddyNamePos  = { 26.0f, 604.0f };
        }

        namespace colors
        {
            constexpr ImU32 kWindowBg      = IM_COL32( 18,  25,  25, 115);
            constexpr ImU32 kBlack         = IM_COL32(  5,   6,   7, 224); // title bar, tiles
            constexpr ImU32 kTitleBar      = IM_COL32(  5,   6,   7, 200);
            constexpr ImU32 kTileHovered   = IM_COL32( 44,  44,  50, 230);
            constexpr ImU32 kTileActive    = IM_COL32( 70,  70,  78, 240);
            constexpr ImU32 kTileSelected  = IM_COL32(246, 246, 246, 255);
            constexpr ImU32 kTextSelected  = IM_COL32( 14,  14,  14, 255);
            constexpr ImU32 kText          = IM_COL32(255, 255, 255, 255);
            constexpr ImU32 kTextDisabled  = IM_COL32(255, 255, 255, 120);

            constexpr ImU32 kBox           = IM_COL32(105,  97,  95, 235);
            constexpr ImU32 kBoxHovered    = IM_COL32(128, 120, 118, 240);
            constexpr ImU32 kBoxActive     = IM_COL32( 88,  81,  79, 245);
            constexpr ImU32 kBoxCheck      = IM_COL32(252, 252, 252, 255);

            constexpr ImU32 kSliderTrack   = IM_COL32( 74,  74,  75, 255);
            constexpr ImU32 kSliderFill    = IM_COL32(128, 129, 132, 255);
        }

        ImFont* font = nullptr;

        // Pushes the window style for the lifetime of the object, so the host's own style is left untouched.
        struct StyleScope
        {
            StyleScope()
            {
                ImGui::PushFont(font);
                ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, layout::kWindowPadding);
                ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, layout::kFramePadding);
                ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, layout::kItemSpacing);
                ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
                ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
                ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0.0f);
                ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
                ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2(0.5f, 0.5f));
                ImGui::PushStyleColor(ImGuiCol_WindowBg, colors::kWindowBg);
                ImGui::PushStyleColor(ImGuiCol_TitleBg, colors::kTitleBar);
                ImGui::PushStyleColor(ImGuiCol_TitleBgActive, colors::kTitleBar);
                ImGui::PushStyleColor(ImGuiCol_TitleBgCollapsed, colors::kTitleBar);
                ImGui::PushStyleColor(ImGuiCol_Button, colors::kBlack);
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, colors::kTileHovered);
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, colors::kTileActive);
                ImGui::PushStyleColor(ImGuiCol_Text, colors::kText);
                ImGui::PushStyleColor(ImGuiCol_TextDisabled, colors::kTextDisabled);
                }
            ~StyleScope()
            {
                ImGui::PopStyleColor(9);
                ImGui::PopStyleVar(8);
                ImGui::PopFont();
            }
        };

        const char* const kTabNames[Tab_Count] = { "Aimbot", "ESP", "Visuals", "Misc", "Skin", "Thirdperson", "Effects", "Setup" };
        constexpr int kTabColumns = 4;

        ImVec2 WindowOffset(const ImVec2& offset)
        {
            return ImGui::GetWindowPos() + offset;
        }

        // 41 px gray square; when checked a white square is drawn inside it. Label to the right, vertically centered.
        bool CheckBox(const char* label, bool* v, const ImVec2& offset)
        {
            ImGui::SetCursorScreenPos(WindowOffset(offset));
            const ImVec2 p = ImGui::GetCursorScreenPos();
            const ImVec2 label_size = ImGui::CalcTextSize(label);

            ImGui::PushID(label);
            const bool pressed = ImGui::InvisibleButton("##box", ImVec2(layout::kBoxSize + layout::kBoxLabelGap + label_size.x, layout::kBoxSize));
            ImGui::PopID();
            if (pressed)
                *v = !*v;

            ImDrawList* dl = ImGui::GetWindowDrawList();
            const ImU32 box = ImGui::IsItemActive() ? colors::kBoxActive : ImGui::IsItemHovered() ? colors::kBoxHovered : colors::kBox;
            const ImVec2 box_max = p + ImVec2(layout::kBoxSize, layout::kBoxSize);
            dl->AddRectFilled(p, box_max, box);
            if (*v)
                dl->AddRectFilled(p + ImVec2(layout::kBoxInset, layout::kBoxInset), box_max - ImVec2(layout::kBoxInset, layout::kBoxInset), colors::kBoxCheck);
            dl->AddText(ImVec2(p.x + layout::kBoxSize + layout::kBoxLabelGap, p.y + (layout::kBoxSize - label_size.y) * 0.5f + layout::kLabelNudgeY), colors::kText, label);
            return pressed;
        }

        // Flat gray bar, filled up to the value. The value is written under the end of the fill.
        bool BuddySlider(int* v, int v_min, int v_max, const ImVec2& offset, const ImVec2& size)
        {
            const ImVec2 p = WindowOffset(offset);
            ImGui::SetCursorScreenPos(ImVec2(p.x, p.y - layout::kSliderHitPad));
            ImGui::InvisibleButton("##buddy_slider", ImVec2(size.x, size.y + 2.0f * layout::kSliderHitPad));

            bool changed = false;
            if (ImGui::IsItemActive())
            {
                const float t = ImSaturate((ImGui::GetIO().MousePos.x - p.x) / size.x);
                const int value = v_min + (int)(t * (float)(v_max - v_min) + 0.5f);
                if (value != *v)
                {
                    *v = value;
                    changed = true;
                }
            }

            const float t = (float)(*v - v_min) / (float)(v_max - v_min);
            const float fill = IM_ROUND(size.x * ImSaturate(t));
            ImDrawList* dl = ImGui::GetWindowDrawList();
            dl->AddRectFilled(p, p + size, colors::kSliderTrack);
            dl->AddRectFilled(p, ImVec2(p.x + fill, p.y + size.y), colors::kSliderFill);

            char text[16];
            std::snprintf(text, sizeof(text), "%d", *v);
            const float text_w = ImGui::CalcTextSize(text).x;
            const float text_x = ImClamp(p.x + fill - text_w * 0.5f, p.x, p.x + size.x - text_w);
            dl->AddText(ImVec2(text_x, p.y + size.y + layout::kSliderValueGap), colors::kText, text);
            return changed;
        }

        const char* BuddyName(int index)
        {
            if (buddy_name)
                return buddy_name(index);
            if (index == 625)
                return "V25A3: Radiant Buddy";
            static char text[32];
            std::snprintf(text, sizeof(text), "Buddy #%d", index);
            return text;
        }

        void DrawSkinPage()
        {
            State& s = state;

            ImGui::SetCursorScreenPos(WindowOffset(layout::kUnlockPos));
            const ImVec2 button_min = ImGui::GetCursorScreenPos();
            if (ImGui::Button("##unlock_all_skins", layout::kUnlockSize))
                s.unlock_all_skins_clicked = true;
            const char* button_label = "unlock all skins";
            const ImVec2 button_label_size = ImGui::CalcTextSize(button_label);
            ImGui::GetWindowDrawList()->AddText(ImVec2(button_min.x + (layout::kUnlockSize.x - button_label_size.x) * 0.5f,
                button_min.y + (layout::kUnlockSize.y - button_label_size.y) * 0.5f + layout::kUnlockTextNudgeY), colors::kText, button_label);

            CheckBox("collection skin changer", &s.collection_skin_changer, layout::kCollectionPos);
            CheckBox("eject shells", &s.eject_shells, layout::kEjectPos);
            CheckBox("buy binds", &s.buy_binds, layout::kBuyBindsPos);
            CheckBox("buddy", &s.buddy, layout::kBuddyPos);

            ImGui::SetCursorScreenPos(WindowOffset(layout::kBuddyLabelPos));
            ImGui::TextUnformatted("buddy");

            BuddySlider(&s.buddy_index, 0, kBuddyMax, layout::kSliderPos, layout::kSliderSize);

            ImGui::SetCursorScreenPos(WindowOffset(layout::kBuddyNamePos));
            ImGui::TextUnformatted(BuddyName(s.buddy_index));
        }

        void DrawEmptyPage()
        {
            ImGui::TextDisabled("Nothing here yet - assign tilemenu::pages[%d].", state.tab);
        }

        void DrawTabs()
        {
            const float width = (ImGui::GetContentRegionAvail().x - layout::kItemSpacing.x * (kTabColumns - 1)) / (float)kTabColumns;
            for (int i = 0; i < Tab_Count; i++)
            {
                if (i % kTabColumns != 0)
                    ImGui::SameLine();

                const bool selected = (i == state.tab);
                if (selected)
                {
                    ImGui::PushStyleColor(ImGuiCol_Button, colors::kTileSelected);
                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, colors::kTileSelected);
                    ImGui::PushStyleColor(ImGuiCol_ButtonActive, colors::kTileSelected);
                    ImGui::PushStyleColor(ImGuiCol_Text, colors::kTextSelected);
                }
                if (ImGui::Button(kTabNames[i], ImVec2(width, 0.0f)))
                    state.tab = i;
                if (selected)
                    ImGui::PopStyleColor(4);
            }
        }
    }

    void Initialize()
    {
        static const ImWchar ranges[] = { 0x0020, 0x017F, 0x2010, 0x203A, 0x20AC, 0x20AC, 0x2122, 0x2122, 0 };
        ImGuiIO& io = ImGui::GetIO();
        font = io.Fonts->AddFontFromMemoryCompressedTTF(roboto_bold_compressed_data, roboto_bold_compressed_size, layout::kFontSize, nullptr, ranges);
    }

    void Render()
    {
        state.unlock_all_skins_clicked = false;
        if (!open)
            return;

        StyleScope style;
        ImGui::SetNextWindowPos(layout::kWindowPos, ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(layout::kWindowSize, ImGuiCond_Always);
        const ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar
                                     | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoSavedSettings;
        if (ImGui::Begin("##tilemenu", nullptr, flags))
        {
            DrawTabs();

            const PageFn page = pages[state.tab];
            if (page)
                page();
            else if (state.tab == Tab_Skin)
                DrawSkinPage();
            else
                DrawEmptyPage();
        }
        ImGui::End();
    }

    void DrawDemoBackdrop()
    {
        const ImVec2 size = ImGui::GetIO().DisplaySize;
        ImGui::GetBackgroundDrawList()->AddRectFilledMultiColor(ImVec2(0.0f, 0.0f), size,
            IM_COL32(190, 70, 55, 255), IM_COL32(40, 42, 48, 255), IM_COL32(28, 30, 34, 255), IM_COL32(150, 80, 65, 255));
    }
}
