#include "style.hpp"

namespace menu::style
{
    void Apply()
    {
        ImGuiStyle& st = ImGui::GetStyle();
        st.WindowRounding = 0.0f;
        st.WindowBorderSize = 0.0f;
        st.ChildBorderSize = 0.0f;
        st.PopupBorderSize = 1.0f;
        st.FrameRounding = 3.0f;
        st.PopupRounding = 3.0f;
        st.GrabRounding = 3.0f;
        st.ScrollbarRounding = 3.0f;
        st.ScrollbarSize = 8.0f;
        st.GrabMinSize = 8.0f;

        const ImVec4 content    = ImGui::ColorConvertU32ToFloat4(kContentBg);
        const ImVec4 sidebar    = ImGui::ColorConvertU32ToFloat4(kSidebarBg);
        const ImVec4 active_bg  = ImGui::ColorConvertU32ToFloat4(kSidebarActiveBg);
        const ImVec4 dark       = ImGui::ColorConvertU32ToFloat4(kDark);
        const ImVec4 accent     = ImGui::ColorConvertU32ToFloat4(kAccent);
        const ImVec4 accent_hi  = ImVec4(0.43f, 0.20f, 1.00f, 1.00f);
        const ImVec4 accent_dim = ImGui::ColorConvertU32ToFloat4(kTabGradientBot);
        const ImVec4 text       = ImGui::ColorConvertU32ToFloat4(kText);
        const ImVec4 none       = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);

        ImVec4* c = st.Colors;
        c[ImGuiCol_Text]                 = text;
        c[ImGuiCol_TextDisabled]         = ImGui::ColorConvertU32ToFloat4(kIcon);
        c[ImGuiCol_WindowBg]             = content;
        c[ImGuiCol_ChildBg]              = none;
        c[ImGuiCol_PopupBg]              = ImGui::ColorConvertU32ToFloat4(kPanelBg);
        c[ImGuiCol_Border]               = active_bg;
        c[ImGuiCol_BorderShadow]         = none;
        c[ImGuiCol_FrameBg]              = active_bg;
        c[ImGuiCol_FrameBgHovered]       = ImGui::ColorConvertU32ToFloat4(kFrameBgHovered);
        c[ImGuiCol_FrameBgActive]        = ImGui::ColorConvertU32ToFloat4(kFrameBgHovered);
        c[ImGuiCol_TitleBg]              = dark;
        c[ImGuiCol_TitleBgActive]        = dark;
        c[ImGuiCol_TitleBgCollapsed]     = dark;
        c[ImGuiCol_MenuBarBg]            = sidebar;
        c[ImGuiCol_ScrollbarBg]          = none;
        c[ImGuiCol_ScrollbarGrab]        = active_bg;
        c[ImGuiCol_ScrollbarGrabHovered] = ImGui::ColorConvertU32ToFloat4(kIcon);
        c[ImGuiCol_ScrollbarGrabActive]  = accent;
        c[ImGuiCol_CheckMark]            = text;
        c[ImGuiCol_SliderGrab]           = accent;
        c[ImGuiCol_SliderGrabActive]     = accent_hi;
        c[ImGuiCol_Button]               = sidebar;
        c[ImGuiCol_ButtonHovered]        = active_bg;
        c[ImGuiCol_ButtonActive]         = accent_dim;
        c[ImGuiCol_Header]               = accent_dim;
        c[ImGuiCol_HeaderHovered]        = active_bg;
        c[ImGuiCol_HeaderActive]         = accent;
        c[ImGuiCol_Separator]            = dark;
        c[ImGuiCol_SeparatorHovered]     = accent_dim;
        c[ImGuiCol_SeparatorActive]      = accent;
        c[ImGuiCol_ResizeGrip]           = none;
        c[ImGuiCol_ResizeGripHovered]    = accent_dim;
        c[ImGuiCol_ResizeGripActive]     = accent;
        c[ImGuiCol_Tab]                  = content;
        c[ImGuiCol_TabHovered]           = active_bg;
        c[ImGuiCol_TabSelected]          = accent_dim;
        c[ImGuiCol_TabSelectedOverline]  = accent;
        c[ImGuiCol_TabDimmed]            = content;
        c[ImGuiCol_TabDimmedSelected]    = sidebar;
        c[ImGuiCol_TextSelectedBg]       = ImVec4(accent.x, accent.y, accent.z, 0.35f);
        c[ImGuiCol_NavCursor]            = accent;
        c[ImGuiCol_ModalWindowDimBg]     = ImVec4(0.0f, 0.0f, 0.0f, 0.55f);
    }
}
