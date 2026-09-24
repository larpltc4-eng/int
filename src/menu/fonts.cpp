#include "fonts.hpp"

#include "icons.hpp"
#include "style.hpp"

#include "fonts/fa_solid_900.h"
#include "fonts/fa_solid_900_v5_3_eye_slash.h"
#include "fonts/montserrat_medium.h"
#include "fonts/montserrat_semibold.h"

namespace menu::fonts
{
    void Load()
    {
        ImFontAtlas* atlas = ImGui::GetIO().Fonts;

        // Basic Latin + Latin-1 + Latin Extended-A (Polish letters) + general punctuation.
        static const ImWchar text_ranges[] = { 0x0020, 0x017F, 0x2010, 0x203A, 0x20AC, 0x20AC, 0x2122, 0x2122, 0 };
        static const ImWchar icon_ranges[] = { ICON_MIN_FA, ICON_MAX_FA, 0 };

        // The oversampling values and raster sizes below reproduce the reference screenshot pixel for pixel.
        ImFontConfig cfg;
        cfg.OversampleH = 3;
        cfg.OversampleV = 1;
        regular = atlas->AddFontFromMemoryCompressedTTF(montserrat_medium_compressed_data, montserrat_medium_compressed_size, style::kFontSize, &cfg, text_ranges);

        cfg.OversampleH = 1;
        tab = atlas->AddFontFromMemoryCompressedTTF(montserrat_semibold_compressed_data, montserrat_semibold_compressed_size, 28.0f, &cfg, text_ranges);

        cfg.OversampleH = 2;
        footer = atlas->AddFontFromMemoryCompressedTTF(montserrat_medium_compressed_data, montserrat_medium_compressed_size, 15.0f, &cfg, text_ranges);

        // The reference uses the older (narrower) eye-slash from Font Awesome 5.3. It is added first because
        // when fonts are merged the first source providing a glyph wins; everything else comes from 5.15.4.
        static const ImWchar eye_slash_range[] = { 0xf070, 0xf070, 0 };
        ImFontConfig icon_cfg;
        icon_cfg.OversampleH = 2;
        icon_cfg.OversampleV = 1;
        icons = atlas->AddFontFromMemoryCompressedTTF(fa_solid_900_v5_3_eye_slash_compressed_data, fa_solid_900_v5_3_eye_slash_compressed_size, style::kIconFontSize, &icon_cfg, eye_slash_range);
        icon_cfg.MergeMode = true;
        atlas->AddFontFromMemoryCompressedTTF(fa_solid_900_compressed_data, fa_solid_900_compressed_size, style::kIconFontSize, &icon_cfg, icon_ranges);
    }
}
