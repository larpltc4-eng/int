// UziClicker: native Win32 window + DirectX 11 (based on imgui/examples/example_win32_directx11).
// Fixed size window like the original: no resizing and a greyed out maximize button.

#include "imgui.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.h"
#include "uzi/menu.hpp"

#include <d3d11.h>

static ID3D11Device*           g_device       = nullptr;
static ID3D11DeviceContext*    g_context      = nullptr;
static IDXGISwapChain*         g_swapChain    = nullptr;
static ID3D11RenderTargetView* g_rtv          = nullptr;
static bool                    g_occluded     = false;
static UINT                    g_resizeWidth  = 0;
static UINT                    g_resizeHeight = 0;
static float                   g_pendingScale = 0.0f; // set by WM_DPICHANGED

static constexpr DWORD kWindowStyle = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;

static bool CreateDeviceD3D(HWND hwnd);
static void CleanupDeviceD3D();
static void CreateRenderTarget();
static void CleanupRenderTarget();
static LRESULT WINAPI WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

int WINAPI WinMain(HINSTANCE instance, HINSTANCE, LPSTR, int)
{
    // Per-monitor DPI aware: the menu is drawn in physical pixels and scaled by the monitor's DPI
    // (624x415 at 100%), instead of being stretched (blurry) by Windows.
    ImGui_ImplWin32_EnableDpiAwareness();
    const HMONITOR monitor = ::MonitorFromPoint(POINT{ 0, 0 }, MONITOR_DEFAULTTOPRIMARY);
    const float scale = ImGui_ImplWin32_GetDpiScaleForMonitor(monitor);

    WNDCLASSEXW wc = {};
    wc.cbSize = sizeof(wc);
    wc.style = CS_CLASSDC;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = instance;
    wc.hIcon = ::LoadIcon(nullptr, IDI_APPLICATION);
    wc.hCursor = ::LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = static_cast<HBRUSH>(::GetStockObject(BLACK_BRUSH));
    wc.lpszClassName = L"UziClicker";
    ::RegisterClassExW(&wc);

    RECT rect = { 0, 0, static_cast<LONG>(uzi::kWidth * scale + 0.5f), static_cast<LONG>(uzi::kHeight * scale + 0.5f) };
    ::AdjustWindowRectEx(&rect, kWindowStyle, FALSE, 0);
    const int width = rect.right - rect.left;
    const int height = rect.bottom - rect.top;
    RECT work;
    ::SystemParametersInfoW(SPI_GETWORKAREA, 0, &work, 0);
    const int x = work.left + (work.right - work.left - width) / 2;
    const int y = work.top + (work.bottom - work.top - height) / 2;
    HWND hwnd = ::CreateWindowExW(0, wc.lpszClassName, L"UziClicker", kWindowStyle, x, y, width, height, nullptr, nullptr, instance, nullptr);

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
    uzi::Setup(scale);
    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(g_device, g_context);

    const float clear_color[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
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

        if (g_occluded && g_swapChain->Present(0, DXGI_PRESENT_TEST) == DXGI_STATUS_OCCLUDED)
        {
            ::Sleep(10);
            continue;
        }
        g_occluded = false;

        if (g_resizeWidth != 0 && g_resizeHeight != 0)
        {
            CleanupRenderTarget();
            g_swapChain->ResizeBuffers(0, g_resizeWidth, g_resizeHeight, DXGI_FORMAT_UNKNOWN, 0);
            g_resizeWidth = g_resizeHeight = 0;
            CreateRenderTarget();
        }

        if (g_pendingScale > 0.0f) // moved to a monitor with another DPI: rebuild fonts and sizes
        {
            uzi::Setup(g_pendingScale);
            ImGui_ImplDX11_InvalidateDeviceObjects(); // recreated (with the new font texture) by NewFrame
            g_pendingScale = 0.0f;
        }

        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        uzi::Render();

        ImGui::Render();
        g_context->OMSetRenderTargets(1, &g_rtv, nullptr);
        g_context->ClearRenderTargetView(g_rtv, clear_color);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        const HRESULT hr = g_swapChain->Present(1, 0); // vsync
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

    const D3D_FEATURE_LEVEL levels[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0 };
    D3D_FEATURE_LEVEL level;
    HRESULT res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, levels, 2, D3D11_SDK_VERSION,
                                                &sd, &g_swapChain, &g_device, &level, &g_context);
    if (res == DXGI_ERROR_UNSUPPORTED) // no hardware support: WARP software driver
        res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_WARP, nullptr, 0, levels, 2, D3D11_SDK_VERSION,
                                            &sd, &g_swapChain, &g_device, &level, &g_context);
    if (res != S_OK)
        return false;

    CreateRenderTarget();
    return true;
}

static void CleanupDeviceD3D()
{
    CleanupRenderTarget();
    if (g_swapChain) { g_swapChain->Release(); g_swapChain = nullptr; }
    if (g_context)   { g_context->Release();   g_context = nullptr; }
    if (g_device)    { g_device->Release();    g_device = nullptr; }
}

static void CreateRenderTarget()
{
    ID3D11Texture2D* back_buffer = nullptr;
    g_swapChain->GetBuffer(0, IID_PPV_ARGS(&back_buffer));
    g_device->CreateRenderTargetView(back_buffer, nullptr, &g_rtv);
    back_buffer->Release();
}

static void CleanupRenderTarget()
{
    if (g_rtv) { g_rtv->Release(); g_rtv = nullptr; }
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

static LRESULT WINAPI WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {
    case WM_SIZE:
        if (wParam == SIZE_MINIMIZED)
            return 0;
        g_resizeWidth = (UINT)LOWORD(lParam);
        g_resizeHeight = (UINT)HIWORD(lParam);
        return 0;
    case WM_DPICHANGED:
    {
        const RECT* suggested = reinterpret_cast<const RECT*>(lParam);
        ::SetWindowPos(hwnd, nullptr, suggested->left, suggested->top, suggested->right - suggested->left, suggested->bottom - suggested->top,
                       SWP_NOZORDER | SWP_NOACTIVATE);
        g_pendingScale = HIWORD(wParam) / 96.0f;
        return 0;
    }
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU) // disable the ALT application menu
            return 0;
        break;
    case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0;
    }
    return ::DefWindowProcW(hwnd, msg, wParam, lParam);
}
