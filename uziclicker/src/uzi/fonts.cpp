#include "fonts.hpp"

#include "style.hpp"

#include "fonts/poppins_bold.h"
#include "fonts/poppins_medium.h"
#include "fonts/poppins_regular.h"

namespace uzi::fonts
{
    namespace
    {
        // Poppins has ascender 1050, descender -350 and 1000 units per em. ImGui scales a font so that
        // ascender - descender equals the requested size, so an em size of N px is requested as N * 1.4.
        // The result is rounded to whole pixels: Dear ImGui 1.92+ only bakes whole pixel sizes and would
        // stretch (blur) anything else, rounding keeps the text sharp and identical on 1.91 and 1.92.
        constexpr float kEmToImGui = 1.4f;

        // `weight`: RasterizerMultiply. The original is drawn by GDI, whose text comes out a bit heavier than
        // ImGui's; 1.2 matches the amount of "ink" of the Regular and Bold texts on the screenshot.
        ImFont* Add(const char* data_base85, float em_size, float scale, float weight)
        {
            // Basic Latin + Latin-1 + Latin Extended-A (Polish letters) + general punctuation.
            static const ImWchar ranges[] = { 0x0020, 0x017F, 0x2010, 0x203A, 0x20AC, 0x20AC, 0x2122, 0x2122, 0 };
            ImFontConfig cfg;
            cfg.OversampleH = 3;
            cfg.OversampleV = 1;
            cfg.RasterizerMultiply = weight;
            const float size = static_cast<float>(static_cast<int>(em_size * kEmToImGui * scale + 0.5f));
            return ImGui::GetIO().Fonts->AddFontFromMemoryCompressedBase85TTF(data_base85, size, &cfg, ranges);
        }
    }

    void Load(float scale)
    {
        ImGui::GetIO().Fonts->Clear();
        text    = Add(poppins_regular_compressed_data_base85, style::kTextSize, scale, 1.2f); // first font = default font
        sidebar = Add(poppins_regular_compressed_data_base85, style::kSidebarSize, scale, 1.2f);
        title   = Add(poppins_medium_compressed_data_base85, style::kTitleSize, scale, 1.0f);
        toggle  = Add(poppins_bold_compressed_data_base85, style::kToggleSize, scale, 1.2f);
    }
}
