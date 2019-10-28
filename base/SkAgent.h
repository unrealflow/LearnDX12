#pragma once
#include "SkBase.h"
#include "SkFence.h"

class SkAgent
{
private:
    SkBase *base;
    SkFence fence;

public:
    void Init(SkBase *initBase)
    {
        base = initBase;
        fence.Create(base);
    }
    SkBase *GetBase()
    {
        return base;
    }
    ComPtr<ID3D12GraphicsCommandList> BeginCmd()
    {
        ComPtr<ID3D12GraphicsCommandList> cmd;
        SK_CHECK(base->device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, base->cmdPool.Get(), nullptr, IID_PPV_ARGS(&cmd)));
        return cmd;
    }
    template <typename T>
    ComPtr<T> BeginCmd()
    {
        ComPtr<T> cmd;
        SK_CHECK(base->device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, base->cmdPool.Get(), nullptr, IID_PPV_ARGS(&cmd)));
        return cmd;
    }
    void FlushCmd(ComPtr<ID3D12GraphicsCommandList> &cmd)
    {
        SK_CHECK(cmd->Close());
        ID3D12CommandList *ppCommandLists[] = {cmd.Get()};
        base->cmdQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
        fence.Signal();
        fence.Wait();
    }
    HRESULT CreateBuffer(D3D12_HEAP_TYPE HeapType,
                         D3D12_HEAP_FLAGS HeapFlags,
                         uint32_t BufferSize,
                         D3D12_RESOURCE_STATES InitialResourceState,
                         const D3D12_CLEAR_VALUE *pOptimizedClearValue,
                         SkBuffer *buffer)
    {
        SK_N_NULL(base->device.Get());
        buffer->bufSize = BufferSize;
        return base->device->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(HeapType),
            HeapFlags,
            &CD3DX12_RESOURCE_DESC::Buffer(BufferSize),
            InitialResourceState,
            pOptimizedClearValue,
            IID_PPV_ARGS(&(buffer->buf)));
    }
};
