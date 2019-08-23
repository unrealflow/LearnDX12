#pragma once
#include "SkTools.h"

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
    ComPtr<ID3D12DescriptorHeap> dsvHeap;
    uint32_t rtvDesSize;
    uint32_t srvDesSize;
    uint32_t dsvDesSize;
    ComPtr<ID3D12PipelineState> pipelineState;
    // ComPtr<ID3D12GraphicsCommandList> cmdList;
    HANDLE fenceEvent;
    ComPtr<ID3D12Fence> fence;
    UINT64 fenceValue;
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

    ComPtr<ID3D12GraphicsCommandList> BeginCmd()
    {
        ComPtr<ID3D12GraphicsCommandList> cmd;
        SK_CHECK(this->device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, this->cmdPool.Get(), nullptr, IID_PPV_ARGS(&cmd)));
        return cmd;
    }
    template <typename T>
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
    HRESULT CreateBuffer(D3D12_HEAP_TYPE HeapType,
                         D3D12_HEAP_FLAGS HeapFlags,
                         uint32_t BufferSize,
                         D3D12_RESOURCE_STATES InitialResourceState,
                         const D3D12_CLEAR_VALUE *pOptimizedClearValue,
                         SkBuffer *buffer)
    {
        SK_N_NULL(this->device.Get());
        buffer->bufSize = BufferSize;
        return this->device->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(HeapType),
            HeapFlags,
            &CD3DX12_RESOURCE_DESC::Buffer(BufferSize),
            InitialResourceState,
            pOptimizedClearValue,
            IID_PPV_ARGS(&(buffer->buf)));
    }
};
