// Cross-platform host for CipherX: undecorated 840x600 GLFW window + OpenGL 3 (Linux / macOS / Windows).
// Drag the top bar to move the window, the X closes it.

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "cipherx/cipherx.hpp"
#include "cipherx/style.hpp"

#include <GLFW/glfw3.h>

#include <cstdio>

int main(int, char**)
{
    glfwSetErrorCallback([](int error, const char* description) { std::fprintf(stderr, "GLFW error %d: %s\n", error, description); });
    if (!glfwInit())
        return 1;

#if defined(__APPLE__)
    const char* glsl_version = "#version 150";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#else
    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
#endif
    glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    GLFWwindow* window = glfwCreateWindow((int)cipherx::style::kWindowSize.x, (int)cipherx::style::kWindowSize.y, "CipherX", nullptr, nullptr);
    if (!window)
        return 1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::GetIO().IniFilename = nullptr;
    cipherx::Initialize();
    cipherx::movable = false; // the OS window is moved instead (below)

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    const ImVec4 bg = ImGui::ColorConvertU32ToFloat4(cipherx::style::kWindowBg);
    bool dragging = false;
    double drag_x = 0.0, drag_y = 0.0; // cursor position in the window when the drag started
    while (!glfwWindowShouldClose(window) && cipherx::open)
    {
        glfwPollEvents();
        if (glfwGetWindowAttrib(window, GLFW_ICONIFIED) != 0)
        {
            ImGui_ImplGlfw_Sleep(10);
            continue;
        }

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Moving the window by the cursor offset keeps the cursor at the same place in the window.
        double cursor_x, cursor_y;
        glfwGetCursorPos(window, &cursor_x, &cursor_y);
        if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && cipherx::IsDragArea((float)cursor_x, (float)cursor_y))
        {
            dragging = true;
            drag_x = cursor_x;
            drag_y = cursor_y;
        }
        if (dragging && !ImGui::IsMouseDown(ImGuiMouseButton_Left))
            dragging = false;
        if (dragging && (cursor_x != drag_x || cursor_y != drag_y))
        {
            int window_x, window_y;
            glfwGetWindowPos(window, &window_x, &window_y);
            glfwSetWindowPos(window, window_x + (int)(cursor_x - drag_x), window_y + (int)(cursor_y - drag_y));
        }

        cipherx::Render();

        ImGui::Render();
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        glViewport(0, 0, width, height);
        glClearColor(bg.x, bg.y, bg.z, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
