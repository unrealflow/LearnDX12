#pragma once
#include "SkTools.h"

using Microsoft::WRL::ComPtr;

class SkBase
{
public:
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
    ComPtr<ID3D12DescriptorHeap> desHeap;
    uint32_t desSize;
    ComPtr<ID3D12PipelineState> pipelineState;
    // ComPtr<ID3D12GraphicsCommandList> cmdList;
    HANDLE fenceEvent;
    ComPtr<ID3D12Fence> fence;
    UINT64 fenceValue;
    bool useWarpDevice = false;

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
