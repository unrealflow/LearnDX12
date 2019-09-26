#pragma once
#include "SkTools.h"
#include "SkCamera.h"
class SkBase
{
public:
    float timer;
    float delta;
    uint32_t width;
    uint32_t height;
    std::string name;
    HWND hwnd = nullptr;
    HINSTANCE hInstance;
    int iCmdShow;
    const uint32_t imageCount = 2;
    const DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM;
    const DXGI_FORMAT depthFormat = DXGI_FORMAT_D32_FLOAT;
    const float clearColor[4] = {0.0f, 0.0f, 0.0f, 0.0f};
    uint32_t imageIndex;
    ComPtr<IDXGISwapChain3> swapChain;
    ComPtr<ID3D12Device> device;
    std::vector<ComPtr<ID3D12Resource>> renderTargets;
    ComPtr<ID3D12CommandAllocator> cmdPool;
    ComPtr<ID3D12CommandQueue> cmdQueue;
    SkHeap *heap = nullptr;
    ComPtr<ID3D12Resource> depthTarget;
    ComPtr<ID3D12PipelineState> pipelineState;
    // ComPtr<ID3D12GraphicsCommandList> cmdList;
    SkCamera cam;
    // bool useWarpDevice = false;

    ComPtr<ID3D12RootSignature> rootSignature;
    SkInclude include;

    HWND GetHwnd()
    {
        if (hwnd == nullptr)
        {
            throw std::runtime_error("Window not Init!");
        }
        return hwnd;
    }
};
class SkCallback
{
public:
    virtual void WinProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) = 0;
};