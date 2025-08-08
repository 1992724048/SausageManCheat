#pragma once
#include "../pch.h"

extern auto ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) -> LRESULT;

class DXHook {
    using IDXGISwapChainPresent = HRESULT(__stdcall*)(IDXGISwapChain* _p_swap_chain, UINT _sync_interval, UINT _flags);

public:
    inline static TEvent<ID3D11DeviceContext*> on_render;
    inline static TEvent<HWND, UINT, WPARAM, LPARAM> wndproc_event;
    inline static TEvent<HWND, ID3D11Device*, ID3D11DeviceContext*, IDXGISwapChain*> initialize_event;

    static auto initialize() -> void {
        find_direct11_present();
    }

    inline static glm::vec2 size;
private:
    inline static IDXGISwapChainPresent present;
    inline static ID3D11Device* p_device = nullptr;
    inline static ID3D11DeviceContext* p_context = nullptr;
    inline static IDXGISwapChain* p_swap_chain = nullptr;
    inline static ID3D11RenderTargetView* main_render_target_view;
    inline static WNDPROC original_wndproc_handler;
    inline static HWND hwnd;

    static auto find_direct11_present() -> IDXGISwapChainPresent {
        WNDCLASSEX wc{};
        wc.cbSize = sizeof(wc);
        wc.lpfnWndProc = DefWindowProc;
        wc.lpszClassName = TEXT("Class");

        if (!RegisterClassEx(&wc)) {
            return nullptr;
        }

        const HWND hwnd = CreateWindow(wc.lpszClassName, TEXT(""), WS_DISABLED, 0, 0, 0, 0, NULL, NULL, NULL, nullptr);

        IDXGISwapChain* swap_chain;

        D3D_FEATURE_LEVEL feature_level;
        DXGI_SWAP_CHAIN_DESC swap_chain_desc;
        ZeroMemory(&swap_chain_desc, sizeof(swap_chain_desc));
        swap_chain_desc.BufferCount = 1;
        swap_chain_desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        swap_chain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swap_chain_desc.OutputWindow = hwnd;
        swap_chain_desc.SampleDesc.Count = 1;
        swap_chain_desc.Windowed = (GetWindowLong(hwnd, GWL_STYLE) & WS_POPUP) != 0 ? FALSE : TRUE;
        swap_chain_desc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
        swap_chain_desc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
        swap_chain_desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

        ID3D11DeviceContext* context = nullptr;
        ID3D11Device* device = nullptr;

        if (FAILED(D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_WARP, nullptr, 0, nullptr, 1, D3D11_SDK_VERSION, &swap_chain_desc, &swap_chain, &device, &feature_level, &context)) && FAILED(
                D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, nullptr, 0, D3D11_SDK_VERSION, &swap_chain_desc, &swap_chain, &device, &feature_level, &context))) {
            DestroyWindow(swap_chain_desc.OutputWindow);
            UnregisterClass(wc.lpszClassName, GetModuleHandle(nullptr));

            return nullptr;
        }

        auto swap_chain_vtable = reinterpret_cast<DWORD_PTR*>(swap_chain);
        swap_chain_vtable = reinterpret_cast<DWORD_PTR*>(swap_chain_vtable[0]);

        DWORD old;
        present = reinterpret_cast<IDXGISwapChainPresent>(swap_chain_vtable[8]);
        VirtualProtect(swap_chain_vtable, 0x1000, PAGE_EXECUTE_READWRITE, &old);
        swap_chain_vtable[8] = reinterpret_cast<DWORD_PTR>(present_hook);
        VirtualProtect(swap_chain_vtable, 0x1000, old, &old);

        device->Release();
        swap_chain->Release();

        DestroyWindow(swap_chain_desc.OutputWindow);
        UnregisterClass(wc.lpszClassName, GetModuleHandle(nullptr));

        return present;
    }

    static auto __stdcall present_hook(IDXGISwapChain* _chain, const UINT _sync_interval, const UINT _flags) -> HRESULT {
        static BOOL initialised = false;

        if (initialised) {
            ImGui_ImplDX11_NewFrame();
            ImGui_ImplWin32_NewFrame();
            ImGui::NewFrame();
            on_render(p_context);
            ImGui::EndFrame();
            ImGui::Render();
            p_context->OMSetRenderTargets(1, &main_render_target_view, nullptr);
            ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
        }

        if (!initialised) {
            const auto result = _chain->GetDevice(__uuidof(p_device), reinterpret_cast<void**>(&p_device));

            if (SUCCEEDED(result)) {
                p_device->GetImmediateContext(&p_context);

                DXGI_SWAP_CHAIN_DESC sd;
                _chain->GetDesc(&sd);

                IMGUI_CHECKVERSION();
                ImGui::CreateContext();
                ImGuiIO& io = ImGui::GetIO();
                (void)io;
                io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
                static const std::string imgui_path = (App::instance().path() / "imgui.ini").string();
                io.IniFilename = imgui_path.data();

                original_wndproc_handler = reinterpret_cast<WNDPROC>(SetWindowLongPtr(sd.OutputWindow, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(wndproc)));

                WINDOWINFO windowInfo = {sizeof(WINDOWINFO)};
                GetWindowInfo(sd.OutputWindow, &windowInfo);
                auto window_width = windowInfo.rcClient.right - windowInfo.rcClient.left;
                auto window_height = windowInfo.rcClient.bottom - windowInfo.rcClient.top;
                size = {window_width, window_height};
                hwnd = sd.OutputWindow;

                ImGui_ImplWin32_Init(sd.OutputWindow);
                ImGui_ImplDX11_Init(p_device, p_context);

                ID3D11Texture2D* p_back_buffer;
                _chain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<LPVOID*>(&p_back_buffer));
                p_device->CreateRenderTargetView(p_back_buffer, nullptr, &main_render_target_view);
                p_back_buffer->Release();

                auto [data, size] = App::instance().read_resource(IDR_TTF1, L"TTF");
                io.Fonts->AddFontFromMemoryTTF(data.get(), size, 16, nullptr, io.Fonts->GetGlyphRangesChineseFull());

                initialize_event(sd.OutputWindow, p_device, p_context, _chain);
                p_swap_chain = _chain;
                initialised = true;
            }
        }

        return present(_chain, _sync_interval, _flags);
    }

    static auto CALLBACK wndproc(const HWND hWnd, const UINT uMsg, const WPARAM wParam, const LPARAM lParam) -> LRESULT {
#ifdef _DEBUG
        ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam);
#endif
        switch (uMsg) {
            case WM_CLOSE:
                if (MessageBoxW(nullptr, L"确定要退出吗?", L"提示", MB_YESNO | MB_ICONQUESTION) == IDYES) {
                    TerminateProcess(GetCurrentProcess(), 0);
                }
            default:
                break;
        }

        wndproc_event(hWnd, uMsg, wParam, lParam);
        return CallWindowProc(original_wndproc_handler, hWnd, uMsg, wParam, lParam);
    }
};
