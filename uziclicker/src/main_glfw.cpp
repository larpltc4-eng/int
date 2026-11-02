// UziClicker: GLFW + OpenGL 3 host (Linux / macOS / Windows), fixed size window like the original.

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "uzi/menu.hpp"

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
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    float scale = 1.0f;
    if (GLFWmonitor* monitor = glfwGetPrimaryMonitor())
        glfwGetMonitorContentScale(monitor, &scale, nullptr);
#if defined(__APPLE__)
    scale = 1.0f; // macOS scales windows itself (Retina framebuffer)
#endif

    GLFWwindow* window = glfwCreateWindow(static_cast<int>(uzi::kWidth * scale + 0.5f), static_cast<int>(uzi::kHeight * scale + 0.5f), uzi::kTitle, nullptr, nullptr);
    if (!window)
        return 1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // vsync

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::GetIO().IniFilename = nullptr;
    uzi::Setup(scale);
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    while (!glfwWindowShouldClose(window))
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

        uzi::Render();

        ImGui::Render();
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        glViewport(0, 0, width, height);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
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
