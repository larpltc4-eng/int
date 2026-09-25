#define IMGUI_DEFINE_MATH_OPERATORS
#include "menu.hpp"

#include "fonts.hpp"
#include "icons.hpp"
#include "style.hpp"
#include "widgets.hpp"

#include "imgui_internal.h"

namespace menu
{
    namespace
    {
        constexpr const char* kBrand     = "Impulze";
        constexpr const char* kBuildDate = "Build Date: " __TIME__ " | " __DATE__;

        // Page contents ------------------------------------------------------
        // Example values for the widget showcase. Replace with your own settings.
        struct State
        {
            bool enabled = true;
            bool watermark = false;
            bool keybinds = true;
            bool performance = false;
            float speed = 42.5f;
            int amount = 3;
            int mode = 0;
            bool filters[4] = { true, false, true, false };
            float accent[4] = { 80 / 255.0f, 0.0f, 1.0f, 1.0f };
            float background[4] = { 21 / 255.0f, 20 / 255.0f, 27 / 255.0f, 0.85f };

            // Placeholder pages: [page][tab]
            bool options[6][4][4] = {};
            float values[6][4] = {};
            int modes[6][4] = {};
            float colors[6][4][4] = {};
        };
        State state;

        constexpr const char* kModes[]   = { "Default", "Smooth", "Instant" };
        constexpr const char* kFilters[] = { "Text", "Icons", "Shadows", "Outline" };

        // Two panels side by side filling the page.
        template <typename Left, typename Right>
        void TwoPanels(const char* left_title, Left&& left, const char* right_title, Right&& right)
        {
            const float half = IM_TRUNC((ImGui::GetContentRegionAvail().x - style::kPanelSpacing) * 0.5f);
            widgets::BeginPanel(left_title, ImVec2(half, 0.0f));
            left();
            widgets::EndPanel();
            ImGui::SameLine(0.0f, style::kPanelSpacing);
            widgets::BeginPanel(right_title);
            right();
            widgets::EndPanel();
        }

        void DrawShowcase()
        {
            TwoPanels("General", [] {
                widgets::Checkbox("Enabled", &state.enabled);
                widgets::Checkbox("Show watermark", &state.watermark);
                widgets::Checkbox("Show keybinds", &state.keybinds);
                widgets::Toggle("Performance mode", &state.performance);
                widgets::Keybind("Menu key", &toggle_key);
                if (widgets::Button("Reset"))
                    state = State();
            }, "Settings", [] {
                widgets::SliderFloat("Speed", &state.speed, 0.0f, 100.0f, "%.1f");
                widgets::SliderInt("Amount", &state.amount, 0, 10);
                widgets::Combo("Mode", &state.mode, kModes, IM_ARRAYSIZE(kModes));
                widgets::MultiCombo("Filters", state.filters, kFilters, IM_ARRAYSIZE(kFilters));
                widgets::ColorEdit("Accent color", state.accent);
                widgets::ColorEdit("Background", state.background);
            });
        }

        void DrawPlaceholder(int page, int tab);

        void DrawAim(int tab)
        {
            if (tab == 0)
                DrawShowcase();
            else
                DrawPlaceholder(1, tab);
        }

        // Pages ----------------------------------------------------------------
        struct Page
        {
            const char* icon;
            const char* name;
            const char* tabs[4];
            int tab_count;
            void (*draw)(int tab);
        };

        void DrawPlayer(int tab)   { DrawPlaceholder(0, tab); }
        void DrawVisuals(int tab)  { DrawPlaceholder(2, tab); }
        void DrawConfigs(int tab)  { DrawPlaceholder(3, tab); }
        void DrawSettings(int tab) { DrawPlaceholder(4, tab); }
        void DrawScripts(int tab)  { DrawPlaceholder(5, tab); }

        const Page kPages[] = {
            { ICON_FA_STREET_VIEW, "Player",   { "General", "Movement", "Misc" },    3, DrawPlayer   },
            { ICON_FA_CROSSHAIRS,  "Aim",      { "General", "Weapons", "Trigger" },  3, DrawAim      },
            { ICON_FA_EYE_SLASH,   "Visuals",  { "Players", "World", "Misc" },       3, DrawVisuals  },
            { ICON_FA_CLOUD,       "Configs",  { "Local", "Cloud" },                 2, DrawConfigs  },
            { ICON_FA_COGS,        "Settings", { "General", "Menu" },                2, DrawSettings },
            { ICON_FA_CODE,        "Scripts",  { "Scripts", "Console" },             2, DrawScripts  },
        };
        constexpr int kPageCount = IM_ARRAYSIZE(kPages);

        int current_page = 1; // crosshairs, as in the reference screenshot
        int current_tab[kPageCount] = {};
        bool sidebar_expanded = false;

        void DrawPlaceholder(int page, int tab)
        {
            bool* options = state.options[page][tab];
            TwoPanels(kPages[page].tabs[tab], [&] {
                widgets::Checkbox("Option 1", &options[0]);
                widgets::Checkbox("Option 2", &options[1]);
                widgets::Checkbox("Option 3", &options[2]);
                widgets::Toggle("Option 4", &options[3]);
            }, "Options", [&] {
                widgets::SliderFloat("Value", &state.values[page][tab], 0.0f, 100.0f, "%.0f");
                widgets::Combo("Mode", &state.modes[page][tab], kModes, IM_ARRAYSIZE(kModes));
                widgets::ColorEdit("Color", state.colors[page][tab]);
            });
        }

        // Layout ---------------------------------------------------------------
        void DrawSidebar(const ImVec2& pos, float width, float expand_t)
        {
            ImGui::SetCursorScreenPos(pos);
            if (widgets::SidebarButton("##hamburger", ICON_FA_BARS, nullptr, false, ImVec2(width, style::kSidebarHeaderHeight), 0.0f))
                sidebar_expanded = !sidebar_expanded;

            const float label_alpha = ImSaturate((expand_t - 0.35f) / 0.65f);
            for (int i = 0; i < kPageCount; i++)
            {
                ImGui::SetCursorScreenPos(ImVec2(pos.x, pos.y + style::kSidebarHeaderHeight + style::kSidebarItemHeight * i));
                ImGui::PushID(i);
                if (widgets::SidebarButton("##page", kPages[i].icon, kPages[i].name, i == current_page, ImVec2(width, style::kSidebarItemHeight), label_alpha))
                    current_page = i;
                ImGui::PopID();
            }
        }

        void DrawTabs(const ImVec2& pos, float right)
        {
            ImDrawList* dl = ImGui::GetWindowDrawList();
            const float line_y = pos.y + style::kTabHeight - style::kTabLineHeight;
            dl->AddRectFilled(ImVec2(pos.x, line_y), ImVec2(right, pos.y + style::kTabHeight), style::kDark, style::kTabLineRounding);

            const Page& page = kPages[current_page];
            int& tab = current_tab[current_page];
            ImGui::SetCursorScreenPos(pos);
            ImGui::PushID(current_page);
            for (int i = 0; i < page.tab_count; i++)
            {
                if (i > 0)
                    ImGui::SameLine(0.0f, 0.0f);
                if (widgets::Tab(page.tabs[i], i == tab))
                    tab = i;
            }
            ImGui::PopID();
        }

        void DrawFooter(const ImVec2& min, const ImVec2& max)
        {
            ImDrawList* dl = ImGui::GetWindowDrawList();
            dl->AddRectFilled(min, max, style::kDark);

            ImFont* font = fonts::footer;
            const float size = style::kFooterFontSize;
            const float text_w = font->CalcTextSizeA(size, FLT_MAX, 0.0f, kBuildDate).x;
            dl->AddText(font, size, ImVec2(min.x + style::kFooterPaddingX, min.y), style::kText, kBrand);
            dl->AddText(font, size, ImVec2(IM_TRUNC(max.x - style::kFooterPaddingRightX - text_w), min.y), style::kText, kBuildDate);
        }
    }

    void Initialize()
    {
        fonts::Load();
        style::Apply();
    }

    void Render()
    {
        if (!open)
            return;

        ImGui::SetNextWindowPos(style::kWindowPos, ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(style::kWindowSize, ImGuiCond_Always);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        const ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;
        const bool visible = ImGui::Begin("##menu", &open, flags);
        ImGui::PopStyleVar(3);
        if (!visible)
        {
            ImGui::End();
            return;
        }
        ImGui::PushFont(fonts::regular);

        ImDrawList* dl = ImGui::GetWindowDrawList();
        const ImVec2 p = ImGui::GetWindowPos();
        const ImVec2 s = style::kWindowSize;
        const float body_h = s.y - style::kFooterHeight;

        const float expand_t = widgets::Animate(ImGui::GetID("##sidebar"), sidebar_expanded ? 1.0f : 0.0f, 12.0f);
        const float sidebar_w = IM_TRUNC(ImLerp(style::kSidebarWidth, style::kSidebarWidthExpanded, expand_t));

        dl->AddRectFilled(p, ImVec2(p.x + sidebar_w, p.y + body_h), style::kSidebarBg);
        dl->AddRectFilled(ImVec2(p.x + sidebar_w, p.y), ImVec2(p.x + s.x, p.y + body_h), style::kContentBg);

        DrawSidebar(p, sidebar_w, expand_t);

        const float content_x = p.x + sidebar_w + style::kContentPadding;
        const float content_r = p.x + s.x - style::kContentPadding;
        DrawTabs(ImVec2(content_x, p.y), content_r);

        const float page_y = p.y + style::kTabHeight + style::kPageSpacing;
        ImGui::SetCursorScreenPos(ImVec2(content_x, page_y));
        ImGui::BeginChild("##page", ImVec2(content_r - content_x, p.y + body_h - style::kContentPadding - page_y), ImGuiChildFlags_None, ImGuiWindowFlags_NoBackground);
        kPages[current_page].draw(current_tab[current_page]);
        ImGui::EndChild();

        DrawFooter(ImVec2(p.x, p.y + body_h), ImVec2(p.x + s.x, p.y + s.y));

        ImGui::PopFont();
        ImGui::End();
    }
}
