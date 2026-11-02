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

        ImGuiStyle& st = ImGui::GetStyle();
        st = ImGuiStyle();
        st.WindowPadding    = ImVec2(0.0f, 0.0f);
        st.WindowBorderSize = 0.0f;
        st.WindowRounding   = 0.0f;
        st.PopupBorderSize  = 0.0f;
        st.FrameBorderSize  = 0.0f;

        ImVec4* c = st.Colors;
        c[ImGuiCol_WindowBg]       = ImGui::ColorConvertU32ToFloat4(style::kBackground);
        c[ImGuiCol_Text]           = ImGui::ColorConvertU32ToFloat4(style::kText);
        c[ImGuiCol_PopupBg]        = ImGui::ColorConvertU32ToFloat4(style::kDropdown);
        c[ImGuiCol_TextSelectedBg] = ImVec4(0.0f, 187 / 255.0f, 1.0f, 0.40f);
        c[ImGuiCol_NavCursor]      = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
    }

    void Render()
    {
        ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
        ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
        const ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings |
                                       ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoNavFocus;
        ImGui::PushFont(fonts::text);
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
    }

    Settings& GetSettings() { return g_settings; }
    Page&     CurrentPage() { return g_page; }
}
