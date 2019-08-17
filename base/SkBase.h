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
    //render target view (RTV) descriptor heap
    ComPtr<ID3D12DescriptorHeap> rtvHeap;
    //shader resource view (SRV) heap
    ComPtr<ID3D12DescriptorHeap> srvHeap;
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

    ComPtr<ID3D12GraphicsCommandList> BeginCmd()
    {
        ComPtr<ID3D12GraphicsCommandList> cmd;
        SK_CHECK(this->device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, this->cmdPool.Get(), nullptr, IID_PPV_ARGS(&cmd)));
        return cmd;
    }
    template<typename T>
    ComPtr<T> BeginCmd()
    {
        ComPtr<T> cmd;
        SK_CHECK(this->device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, this->cmdPool.Get(), nullptr, IID_PPV_ARGS(&cmd)));
        return cmd;
    }
    void FlushCmd(ComPtr<ID3D12GraphicsCommandList> &cmd)
    {
        SK_CHECK(cmd->Close());
        ID3D12CommandList *ppCommandLists[] = {cmd.Get()};
        this->cmdQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
    }
};
