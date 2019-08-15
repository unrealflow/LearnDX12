#pragma once
#include "SkBase.h"
class SkFence
{
private:
    SkBase *base;
    HANDLE fenceEvent;
    ComPtr<ID3D12Fence> fence;
    UINT64 fenceValue;

public:
    //需要base->device和base->cmdQueue
    void Create(SkBase *initBase)
    {
        base = initBase;
        SK_CHECK(base->device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&this->fence)));
        fenceValue = 0;
        this->fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
        if (this->fenceEvent == nullptr)
        {
            SK_CHECK(HRESULT_FROM_WIN32(GetLastError()));
        }
    }
    void Signal()
    {
        fenceValue++;
        SK_CHECK(base->cmdQueue->Signal(this->fence.Get(), fenceValue));
    }
    void Wait()
    {
        // Wait until the previous frame is finished.
        if (this->fence->GetCompletedValue() < fenceValue)
        {
            SK_CHECK(this->fence->SetEventOnCompletion(fenceValue, this->fenceEvent));
            WaitForSingleObject(this->fenceEvent, INFINITE);
        }
        // fprintf(stderr,"%lld...\n",fenceValue);
    }
    static void WaitAllFence(std::vector<SkFence> &fences)
    {
        for (size_t i = 0; i < fences.size(); i++)
        {
            fences[i].Wait();
        }
    }
    static void CleanAllFence(std::vector<SkFence> &fences)
    {
        SkFence::WaitAllFence(fences);
        for (size_t i = 0; i < fences.size(); i++)
        {
            CloseHandle(fences[i].fenceEvent);
        }
    }
};
