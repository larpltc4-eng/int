cmake_minimum_required(VERSION 3.16)
project(uziclicker LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(UZI_IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../external/imgui CACHE PATH "Dear ImGui sources (1.91.x)")

if(WIN32)
    set(_uzi_glfw_default OFF)
else()
    set(_uzi_glfw_default ON)
endif()
option(UZI_BUILD_GLFW "Build the GLFW + OpenGL 3 version" ${_uzi_glfw_default})

# Dear ImGui core: reuses the target of the repository root when built from there.
if(NOT TARGET imgui)
    add_library(imgui STATIC
        ${UZI_IMGUI_DIR}/imgui.cpp
        ${UZI_IMGUI_DIR}/imgui_draw.cpp
        ${UZI_IMGUI_DIR}/imgui_tables.cpp
        ${UZI_IMGUI_DIR}/imgui_widgets.cpp
    )
    target_include_directories(imgui PUBLIC ${UZI_IMGUI_DIR} ${UZI_IMGUI_DIR}/backends)
endif()

# The menu (renderer agnostic) ---------------------------------------------------------
add_library(uzi_menu STATIC
    src/uzi/fonts.cpp
    src/uzi/menu.cpp
    src/uzi/widgets.cpp
)
target_include_directories(uzi_menu PUBLIC src)
target_link_libraries(uzi_menu PUBLIC imgui)
if(MSVC)
    target_compile_options(uzi_menu PRIVATE /W4 /utf-8) # sources are UTF-8 (Polish texts)
else()
    target_compile_options(uzi_menu PRIVATE -Wall -Wextra)
endif()

# Windows: native window + DirectX 11 --------------------------------------------------
if(WIN32)
    add_executable(uziclicker WIN32
        src/main_dx11.cpp
        ${UZI_IMGUI_DIR}/backends/imgui_impl_win32.cpp
        ${UZI_IMGUI_DIR}/backends/imgui_impl_dx11.cpp
    )
    target_link_libraries(uziclicker PRIVATE uzi_menu d3d11 d3dcompiler dxgi)
    if(MSVC)
        set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT uziclicker)
    endif()
endif()

# GLFW + OpenGL 3 ------------------------------------------------------------------------
if(UZI_BUILD_GLFW)
    find_package(OpenGL REQUIRED)
    find_package(glfw3 3.3 REQUIRED)
    add_executable(uziclicker_glfw
        src/main_glfw.cpp
        ${UZI_IMGUI_DIR}/backends/imgui_impl_glfw.cpp
        ${UZI_IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
    )
    target_link_libraries(uziclicker_glfw PRIVATE uzi_menu glfw OpenGL::GL ${CMAKE_DL_LIBS})
endif()
