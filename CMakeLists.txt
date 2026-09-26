cmake_minimum_required(VERSION 3.16)
project(imgui_menu LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(WIN32)
    set(_glfw_default OFF)
else()
    set(_glfw_default ON)
endif()
option(MENU_BUILD_GLFW "Build the GLFW + OpenGL3 example" ${_glfw_default})

set(IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/imgui)

# Dear ImGui core ---------------------------------------------------------------
add_library(imgui STATIC
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
)
target_include_directories(imgui PUBLIC ${IMGUI_DIR} ${IMGUI_DIR}/backends)

# The menu (renderer agnostic) ----------------------------------------------------
add_library(menu STATIC
    src/menu/fonts.cpp
    src/menu/menu.cpp
    src/menu/style.cpp
    src/menu/widgets.cpp
)
target_include_directories(menu PUBLIC src)
target_link_libraries(menu PUBLIC imgui)
if(MSVC)
    target_compile_options(menu PRIVATE /W4 /utf-8)
else()
    target_compile_options(menu PRIVATE -Wall -Wextra)
endif()

# CipherX (renderer agnostic) -------------------------------------------------------
find_package(Threads REQUIRED)
add_library(cipherx STATIC
    src/cipherx/cipherx.cpp
    src/cipherx/fonts.cpp
)
target_include_directories(cipherx PUBLIC src)
target_link_libraries(cipherx PUBLIC imgui Threads::Threads)
if(MSVC)
    target_compile_options(cipherx PRIVATE /W4 /utf-8)
else()
    target_compile_options(cipherx PRIVATE -Wall -Wextra)
endif()

# Windows: Win32 + DirectX 11 ------------------------------------------------------
if(WIN32)
    add_executable(menu_dx11 WIN32
        src/main_dx11.cpp
        ${IMGUI_DIR}/backends/imgui_impl_win32.cpp
        ${IMGUI_DIR}/backends/imgui_impl_dx11.cpp
    )
    target_link_libraries(menu_dx11 PRIVATE menu d3d11 d3dcompiler dxgi)
    if(MSVC)
        set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT menu_dx11)
    endif()

    add_executable(cipherx_dx11 WIN32
        src/cipherx_dx11.cpp
        ${IMGUI_DIR}/backends/imgui_impl_win32.cpp
        ${IMGUI_DIR}/backends/imgui_impl_dx11.cpp
    )
    target_link_libraries(cipherx_dx11 PRIVATE cipherx d3d11 d3dcompiler dxgi)
endif()

# GLFW + OpenGL 3 -------------------------------------------------------------------
if(MENU_BUILD_GLFW)
    find_package(OpenGL REQUIRED)
    find_package(glfw3 3.3 REQUIRED)
    add_executable(menu_glfw
        src/main_glfw.cpp
        ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
        ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
    )
    target_link_libraries(menu_glfw PRIVATE menu glfw OpenGL::GL ${CMAKE_DL_LIBS})

    add_executable(cipherx_glfw
        src/cipherx_glfw.cpp
        ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
        ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
    )
    target_link_libraries(cipherx_glfw PRIVATE cipherx glfw OpenGL::GL ${CMAKE_DL_LIBS})
endif()
