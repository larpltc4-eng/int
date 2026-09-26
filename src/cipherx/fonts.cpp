#include "fonts.hpp"

#include "style.hpp"

#include "imgui.h"
#include "imgui_internal.h"

#include "fonts/jetbrains_mono_regular.h"
#include "fonts/selawik_regular.h"
#include "fonts/selawik_semibold.h"

#include <cstring>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

namespace cipherx::fonts
{
    namespace
    {
        // Basic Latin + Latin-1 + Latin Extended-A (Polish letters) + general punctuation.
        const ImWchar kRanges[] = { 0x0020, 0x017F, 0x2010, 0x203A, 0x20AC, 0x20AC, 0x2122, 0x2122, 0 };

        // hhea ascender/descender given to the embedded fonts, in their own units. ImGui sizes a font by
        // ascender - descender and puts the baseline at the ascender, so these make them match the Windows fonts:
        // Segoe UI is 2210/-514 of 2048 and Selawik already has its advance widths; Consolas is 1884/-514 of 2048
        // with 1126 per character, which for JetBrains Mono (600 of 1000 per character) scales to 1004/-274.
        constexpr int kSegoeAscent = 2210, kSegoeDescent = -514;
        constexpr int kConsolasAscent = 1004, kConsolasDescent = -274;

        ImFont* LoadWindowsFont(const char* file, float size)
        {
#ifdef _WIN32
            wchar_t dir[MAX_PATH];
            const UINT len = ::GetWindowsDirectoryW(dir, MAX_PATH);
            if (len == 0 || len >= MAX_PATH)
                return nullptr;
            char path[MAX_PATH * 3];
            const int n = ::WideCharToMultiByte(CP_UTF8, 0, dir, (int)len, path, (int)sizeof(path), nullptr, nullptr);
            if (n <= 0)
                return nullptr;
            ImFormatString(path + n, sizeof(path) - n, "\\Fonts\\%s", file);

            size_t data_size = 0;
            void* data = ImFileLoadToMemory(path, "rb", &data_size); // freed by the atlas
            if (!data)
                return nullptr;
            return ImGui::GetIO().Fonts->AddFontFromMemoryTTF(data, (int)data_size, size, nullptr, kRanges);
#else
            (void)file;
            (void)size;
            return nullptr;
#endif
        }

        // Overwrites the hhea ascender/descender/line gap of the font just added (before the atlas is built).
        void SetVerticalMetrics(ImFontConfig& src, int ascent, int descent)
        {
            unsigned char* data = (unsigned char*)src.FontData;
            const int num_tables = (data[4] << 8) | data[5];
            for (int i = 0; i < num_tables; i++)
            {
                const unsigned char* record = data + 12 + 16 * i;
                if (memcmp(record, "hhea", 4) != 0)
                    continue;
                unsigned char* hhea = data + (((unsigned)record[8] << 24) | ((unsigned)record[9] << 16) | ((unsigned)record[10] << 8) | record[11]);
                const int values[3] = { ascent, descent, 0 };
                for (int v = 0; v < 3; v++)
                {
                    hhea[4 + v * 2] = (unsigned char)((values[v] >> 8) & 0xFF);
                    hhea[5 + v * 2] = (unsigned char)(values[v] & 0xFF);
                }
                return;
            }
        }

        ImFont* LoadEmbeddedFont(const unsigned int* data, unsigned int data_size, float size, int ascent, int descent)
        {
            ImFontAtlas* atlas = ImGui::GetIO().Fonts;
            ImFont* font = atlas->AddFontFromMemoryCompressedTTF(data, (int)data_size, size, nullptr, kRanges);
            SetVerticalMetrics(atlas->Sources.back(), ascent, descent);
            return font;
        }
    }

    void Load()
    {
        regular = LoadWindowsFont("segoeui.ttf", style::kFontSize);
        if (!regular)
            regular = LoadEmbeddedFont(selawik_regular_compressed_data, selawik_regular_compressed_size, style::kFontSize, kSegoeAscent, kSegoeDescent);

        small = LoadWindowsFont("segoeui.ttf", style::kSmallFontSize);
        if (!small)
            small = LoadEmbeddedFont(selawik_regular_compressed_data, selawik_regular_compressed_size, style::kSmallFontSize, kSegoeAscent, kSegoeDescent);

        title = LoadWindowsFont("seguisb.ttf", style::kTitleFontSize);
        if (!title)
            title = LoadEmbeddedFont(selawik_semibold_compressed_data, selawik_semibold_compressed_size, style::kTitleFontSize, kSegoeAscent, kSegoeDescent);

        mono = LoadWindowsFont("consola.ttf", style::kMonoFontSize);
        if (!mono)
            mono = LoadEmbeddedFont(jetbrains_mono_regular_compressed_data, jetbrains_mono_regular_compressed_size, style::kMonoFontSize, kConsolasAscent, kConsolasDescent);
    }
}
