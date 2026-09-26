// Windows host for CipherX: borderless 840x600 Win32 window + DirectX 11.
// Drag the top bar to move the window, the X closes it.

#include "imgui.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.h"
#include "cipherx/cipherx.hpp"
#include "cipherx/style.hpp"

#include <d3d11.h>
#include <windowsx.h>

static ID3D11Device*           g_device = nullptr;
static ID3D11DeviceContext*    g_context = nullptr;
static IDXGISwapChain*         g_swap_chain = nullptr;
static ID3D11RenderTargetView* g_rtv = nullptr;
static bool                    g_occluded = false;

static bool CreateDeviceD3D(HWND hwnd);
static void CleanupDeviceD3D();
static void CreateRenderTarget();
static void CleanupRenderTarget();
static LRESULT WINAPI WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE, PWSTR, int)
{
    ImGui_ImplWin32_EnableDpiAwareness(); // 1 pixel of the UI = 1 screen pixel, like the reference

    WNDCLASSEXW wc = { sizeof(wc), CS_CLASSDC, WndProc, 0, 0, instance, nullptr, ::LoadCursor(nullptr, IDC_ARROW), nullptr, nullptr, L"cipherx", nullptr };
    ::RegisterClassExW(&wc);

    const int width = (int)cipherx::style::kWindowSize.x;
    const int height = (int)cipherx::style::kWindowSize.y;
    const int x = (::GetSystemMetrics(SM_CXSCREEN) - width) / 2;
    const int y = (::GetSystemMetrics(SM_CYSCREEN) - height) / 2;
    HWND hwnd = ::CreateWindowExW(0, wc.lpszClassName, L"CipherX", WS_POPUP | WS_SYSMENU | WS_MINIMIZEBOX, x, y, width, height, nullptr, nullptr, instance, nullptr);

    if (!CreateDeviceD3D(hwnd))
    {
        CleanupDeviceD3D();
        ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
        return 1;
    }
    ::ShowWindow(hwnd, SW_SHOWDEFAULT);
    ::UpdateWindow(hwnd);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::GetIO().IniFilename = nullptr;
    cipherx::Initialize();
    cipherx::movable = false; // the OS window is moved instead (see WM_NCHITTEST)

    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(g_device, g_context);

    const ImVec4 bg = ImGui::ColorConvertU32ToFloat4(cipherx::style::kWindowBg);
    const float clear_color[4] = { bg.x, bg.y, bg.z, 1.0f };
    bool done = false;
    while (!done)
    {
        MSG msg;
        while (::PeekMessageW(&msg, nullptr, 0U, 0U, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessageW(&msg);
            if (msg.message == WM_QUIT)
                done = true;
        }
        if (done)
            break;

        if (g_occluded && g_swap_chain->Present(0, DXGI_PRESENT_TEST) == DXGI_STATUS_OCCLUDED)
        {
            ::Sleep(10);
            continue;
        }
        g_occluded = false;

        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        cipherx::Render();
        if (!cipherx::open)
            ::PostQuitMessage(0);

        ImGui::Render();
        g_context->OMSetRenderTargets(1, &g_rtv, nullptr);
        g_context->ClearRenderTargetView(g_rtv, clear_color);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        const HRESULT hr = g_swap_chain->Present(1, 0);
        g_occluded = (hr == DXGI_STATUS_OCCLUDED);
    }

    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    CleanupDeviceD3D();
    ::DestroyWindow(hwnd);
    ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
    return 0;
}

static bool CreateDeviceD3D(HWND hwnd)
{
    DXGI_SWAP_CHAIN_DESC sd = {};
    sd.BufferCount = 2;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hwnd;
    sd.SampleDesc.Count = 1;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    const D3D_FEATURE_LEVEL levels[] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0 };
    D3D_FEATURE_LEVEL level;
    HRESULT res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, levels, 2, D3D11_SDK_VERSION, &sd, &g_swap_chain, &g_device, &level, &g_context);
    if (res == DXGI_ERROR_UNSUPPORTED) // fall back to the WARP software driver
        res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_WARP, nullptr, 0, levels, 2, D3D11_SDK_VERSION, &sd, &g_swap_chain, &g_device, &level, &g_context);
    if (res != S_OK)
        return false;

    CreateRenderTarget();
    return true;
}

static void CleanupDeviceD3D()
{
    CleanupRenderTarget();
    if (g_swap_chain) { g_swap_chain->Release(); g_swap_chain = nullptr; }
    if (g_context)    { g_context->Release();    g_context = nullptr; }
    if (g_device)     { g_device->Release();     g_device = nullptr; }
}

static void CreateRenderTarget()
{
    ID3D11Texture2D* back_buffer = nullptr;
    g_swap_chain->GetBuffer(0, IID_PPV_ARGS(&back_buffer));
    g_device->CreateRenderTargetView(back_buffer, nullptr, &g_rtv);
    back_buffer->Release();
}

static void CleanupRenderTarget()
{
    if (g_rtv) { g_rtv->Release(); g_rtv = nullptr; }
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

static LRESULT WINAPI WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam))
        return true;

    switch (msg)
    {
    case WM_NCHITTEST:
    {
        // The top bar behaves like a title bar, so Windows moves the window when it is dragged.
        POINT pt = { GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam) };
        ::ScreenToClient(hwnd, &pt);
        return cipherx::IsDragArea((float)pt.x, (float)pt.y) ? HTCAPTION : HTCLIENT;
    }
    case WM_NCLBUTTONDBLCLK: // no maximize on double click
        return 0;
    case WM_SYSCOMMAND:
        if ((wparam & 0xfff0) == SC_KEYMENU)
            return 0;
        break;
    case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0;
    }
    return ::DefWindowProcW(hwnd, msg, wparam, lparam);
}
