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
    uint32_t imageIndex;
    ComPtr<IDXGISwapChain3> swapChain;
    ComPtr<ID3D12Device> device;
    std::vector<ComPtr<ID3D12Resource>> renderTargets;
    ComPtr<ID3D12CommandAllocator> cmdPool;
    ComPtr<ID3D12CommandQueue> cmdQueue;
    //render target view (RTV) descriptor heap
    ComPtr<ID3D12DescriptorHeap> rtvHeap;
    //shader resource view (SRV) heap
    ComPtr<ID3D12DescriptorHeap> srvHeap;
    ComPtr<ID3D12DescriptorHeap> dsvHeap;
    uint32_t rtvDesSize;
    uint32_t srvDesSize;
    uint32_t dsvDesSize;
    ComPtr<ID3D12Resource> depthTarget;
    ComPtr<ID3D12PipelineState> pipelineState;
    // ComPtr<ID3D12GraphicsCommandList> cmdList;
    SkCamera cam;
    // bool useWarpDevice = false;

    ComPtr<ID3D12RootSignature> rootSignature;

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