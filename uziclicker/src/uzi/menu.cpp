#define IMGUI_DEFINE_MATH_OPERATORS
#include "menu.hpp"

#include "fonts.hpp"
#include "style.hpp"
#include "widgets.hpp"

namespace uzi
{
    namespace
    {
        Settings g_settings;
        Page     g_page = Page::Makro;

        // Texts are copied 1:1 from the original, typos included ("Gardaowanie", "Przełącznie").
        const char* const kPageNames[] = { "Konto", "Makro", "Klepa", "Bindy", "Kopanie", "Dodatki", "Ustawienia", "Nowości" };
        static_assert(IM_ARRAYSIZE(kPageNames) == static_cast<int>(Page::Count), "one name per page");

        const char* const kClickModes[] = { "Trzymanie", "Kliknięcie" }; // order of uzi::ClickMode

        void DrawSidebar()
        {
            using namespace style;
            widgets::Text(fonts::title, kTitlePos.x, kTitlePos.y, kText, kTitle);
            for (int i = 0; i < static_cast<int>(Page::Count); i++)
            {
                const ImRect bb = widgets::Rect(kSidebarX, kSidebarY + kSidebarPitch * i, kSidebarButtonSize.x, kSidebarButtonSize.y);
                if (widgets::SidebarButton(kPageNames[i], static_cast<int>(g_page) == i, bb))
                    g_page = static_cast<Page>(i);
            }
        }

        // "MIN CPS" / "MAX CPS" row: label, slider and the value box next to it.
        bool CpsRow(const char* id, const char* label, float y, int* v, int v_min, int v_max)
        {
            using namespace style;
            ImGui::PushID(id);
            widgets::Text(fonts::text, kColumn1, y + kSliderBaseline, kText, label);
            bool changed = widgets::Slider("##slider", v, v_min, v_max, y);
            const ImRect box = widgets::Rect(kValueBoxX, y + kValueBoxTop, kValueBoxSize.x, kValueBoxSize.y);
            changed |= widgets::ValueBox("##value", v, v_min, v_max, box, kValueBoxBorder, kSliderBaseline - kValueBoxTop);
            ImGui::PopID();
            return changed;
        }

        // One mouse button: header, bind, click mode and the MIN / MAX CPS rows. `y` is the section offset.
        void MouseButtonSection(const char* id, float y, const char* title, const char* short_name, ButtonSettings& b, int cps_min, int cps_max)
        {
            using namespace style;
            ImGui::PushID(id);
            widgets::Label(kColumn1, y + kHeaderBaseline, title, short_name);
            widgets::Label(kColumn2, y + kHeaderBaseline, "Tryb ", "klikania");

            widgets::BindButton("##bind", &b.bind, widgets::Rect(kColumn1, y + kRowTop, kBindWidth, kRowHeight));

            int mode = static_cast<int>(b.mode);
            if (widgets::Dropdown("##mode", &mode, kClickModes, IM_ARRAYSIZE(kClickModes), widgets::Rect(kColumn2, y + kRowTop, kDropdownWidth, kRowHeight)))
                b.mode = static_cast<ClickMode>(mode);

            // MIN never above MAX: moving one past the other drags the other one along.
            if (CpsRow("min", "MIN CPS", y + kSliderY, &b.min_cps, cps_min, cps_max) && b.max_cps < b.min_cps)
                b.max_cps = b.min_cps;
            if (CpsRow("max", "MAX CPS", y + kSliderY + kSliderPitch, &b.max_cps, cps_min, cps_max) && b.min_cps > b.max_cps)
                b.min_cps = b.max_cps;
            ImGui::PopID();
        }

        void DrawMakro()
        {
            using namespace style;
            Settings& s = g_settings;
            MouseButtonSection("lpm", 0.0f, "Lewy przycisk myszy ", "LPM", s.left, kLeftCpsMin, kLeftCpsMax);
            MouseButtonSection("ppm", kSectionPitch, "Prawy przycisk myszy ", "PPM", s.right, kRightCpsMin, kRightCpsMax);

            widgets::Label(kColumn1, kBottomBaseline, "Gardaowanie ", "MS");
            widgets::ValueBox("##block_ms", &s.block_ms, kBlockMsMin, kBlockMsMax, widgets::Rect(kMsBox.x, kMsBox.y, kMsBox.z, kMsBox.w), kBorder, kMsBaseline);

            widgets::Label(kOptionsHeaderX, kBottomBaseline, "Szczegółowe ", "opcje");
            widgets::Toggle("##auto_block", "Automatyczna garda", &s.auto_block, kOptionsX, kToggleX, kOptionsY);
            widgets::Toggle("##w_tap", "Wuwuwanie (W-tap)", &s.w_tap, kOptionsX, kToggleX, kOptionsY + kOptionsPitch);
            widgets::Toggle("##sticky", "Przełącznie stałe", &s.sticky_toggle, kOptionsX, kToggleX, kOptionsY + kOptionsPitch * 2.0f);
        }

        // Pages that are not visible on the reference screenshot. Build them with the widgets:: functions
        // the same way as DrawMakro(), using the coordinates from style.hpp.
        void DrawPlaceholder(Page page)
        {
            widgets::Text(fonts::text, style::kColumn1, style::kHeaderBaseline, style::kText, kPageNames[static_cast<int>(page)]);
        }
    }

    void Setup(float dpi_scale)
    {
        widgets::SetScale(dpi_scale);
        fonts::Load(dpi_scale);
    }

    void Render(bool fill_display)
    {
        ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoNavFocus;
        if (fill_display)
        {
            ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
            ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
            flags |= ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus;
        }
        else
        {
            ImGui::SetNextWindowPos(ImVec2(100.0f, 100.0f), ImGuiCond_FirstUseEver);
            ImGui::SetNextWindowSize(ImVec2(widgets::Px(kWidth), widgets::Px(kHeight)));
        }

        // The style is pushed here instead of being set globally, so other ImGui windows of the app keep theirs.
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_PopupBorderSize, 0.0f);
        ImGui::PushStyleColor(ImGuiCol_WindowBg, style::kBackground);
        ImGui::PushStyleColor(ImGuiCol_Text, style::kText);
        ImGui::PushStyleColor(ImGuiCol_PopupBg, style::kDropdown);
        ImGui::PushStyleColor(ImGuiCol_TextSelectedBg, IM_COL32(0, 187, 255, 102));
        ImGui::PushStyleColor(ImGuiCol_NavCursor, IM_COL32(0, 0, 0, 0));
        int colors = 5;
#if IMGUI_VERSION_NUM >= 19200
        ImGui::PushStyleColor(ImGuiCol_InputTextCursor, style::kText); // new in 1.92
        colors++;
        // 1.92 multiplies the size given to PushFont() by the global font scales, cancel them out
        // (Setup()'s DPI scale is already in the font).
        const ImGuiStyle& st = ImGui::GetStyle();
        ImGui::PushFont(fonts::text, fonts::text->LegacySize / (st.FontScaleMain * st.FontScaleDpi));
#else
        ImGui::PushFont(fonts::text);
#endif
        if (ImGui::Begin("##uziclicker", nullptr, flags))
        {
            DrawSidebar();
            if (g_page == Page::Makro)
                DrawMakro();
            else
                DrawPlaceholder(g_page);

            if (g_settings.particles)
                widgets::Particles(ImGui::GetCurrentWindow()->Rect());
        }
        ImGui::End();
        ImGui::PopFont();
        ImGui::PopStyleColor(colors);
        ImGui::PopStyleVar(5);
    }

    Settings& GetSettings() { return g_settings; }
    Page&     CurrentPage() { return g_page; }
}
