#pragma once
#include "SkBase.h"

class SkCmd
{
private:
    SkBase *base;
    vector<ComPtr<ID3D12GraphicsCommandList>> cmdLists;

    void WaitForPreviousFrame()
    {
        const UINT64 fence = base->fenceValue;
        SK_CHECK(base->cmdQueue->Signal(base->fence.Get(), fence));
        base->fenceValue++;

        // Wait until the previous frame is finished.
        if (base->fence->GetCompletedValue() < fence)
        {
            SK_CHECK(base->fence->SetEventOnCompletion(fence, base->fenceEvent));
            WaitForSingleObject(base->fenceEvent, INFINITE);
        }

        base->imageIndex = base->swapChain->GetCurrentBackBufferIndex();
    }
     void CreateFence()
    {
        {
            SK_CHECK(base->device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&base->fence)));
            base->fenceValue = 1;

            // Create an event handle to use for frame synchronization.
            base->fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
            if (base->fenceEvent == nullptr)
            {
                SK_CHECK(HRESULT_FROM_WIN32(GetLastError()));
            }
        }
    }
public:
    void Init(SkBase *initBase)
    {
        base = initBase;
    }
    void BuildCmdLists()
    {
        fprintf(stderr, "Cmd::BuildCmdLists...\n");
        CreateFence();
        // Command list allocators can only be reset when the associated
        // command lists have finished execution on the GPU; apps should use
        // fences to determine GPU execution progress.
        
        this->cmdLists.resize(base->imageCount);
        for (size_t i = 0; i < this->cmdLists.size(); i++)
        {
             SK_CHECK(base->device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, base->cmdPool.Get(), nullptr, IID_PPV_ARGS(&this->cmdLists[i])));
             this->cmdLists[i]->Close();
        }

        SK_CHECK(base->cmdPool->Reset());
        for (int i = 0; i < this->cmdLists.size(); i++)
        {
            // However, when ExecuteCommandList() is called on a particular command
            // list, that command list can then be reset at any time and must be before
            // re-recording.
           
            if (this->cmdLists[i].Get() == nullptr)
            {
                fprintf(stderr, "Nullptr...\n");
            }
            SK_CHECK(this->cmdLists[i]->Reset(base->cmdPool.Get(), base->pipelineState.Get()));

            // Indicate that the back buffer will be used as a render target.
            this->cmdLists[i]->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(base->renderTargets[i].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

            CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(base->desHeap->GetCPUDescriptorHandleForHeapStart(), i, base->desSize);

            // Record commands.
            const float clearColor[] = {0.0f, 0.2f, 0.4f, 1.0f};
            this->cmdLists[i]->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

            // Indicate that the back buffer will now be used to present.
            this->cmdLists[i]->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(base->renderTargets[i].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

            SK_CHECK(this->cmdLists[i]->Close());
        }
        fprintf(stderr, "Cmd::BuildCmdLists...OK\n");
    }
    void Submit()
    {

        ID3D12CommandList *ppCommandLists[] = {this->cmdLists[base->imageIndex].Get()};
        base->cmdQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

        // Present the frame.
        SK_CHECK(base->swapChain->Present(1, 0));

        WaitForPreviousFrame();
    }
    void CleanUp()
    {
        // Ensure that the GPU is no longer referencing resources that are about to be
        // cleaned up by the destructor.
        WaitForPreviousFrame();

        CloseHandle(base->fenceEvent);
    }
};
