#pragma once
#include "SkBase.h"

class SkHeap
{
private:
    SkBase *base;
    uint32_t rtvCap = 0;
    uint32_t srvCap = 0;
    uint32_t dsvCap = 0;
    ComPtr<ID3D12DescriptorHeap> rtvHeap;
    ComPtr<ID3D12DescriptorHeap> srvHeap;
    ComPtr<ID3D12DescriptorHeap> dsvHeap;

public:
    uint32_t rtvDesSize;
    uint32_t srvDesSize;
    uint32_t dsvDesSize;
    void Init(SkBase *initBase,
              uint32_t rtvCap,
              uint32_t srvCap,
              uint32_t dsvCap)
    {
        base = initBase;
        this->rtvCap = rtvCap;
        this->srvCap = srvCap;
        this->dsvCap = dsvCap;
        if (rtvCap > 0)
        {
            D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
            rtvHeapDesc.NumDescriptors = rtvCap;
            rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
            rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
            SK_CHECK(base->device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&this->rtvHeap)));
            this->rtvDesSize = base->device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
        }
        if (srvCap > 0)
        {
            D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
            srvHeapDesc.NumDescriptors = srvCap;
            srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
            srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
            SK_CHECK(base->device->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&this->srvHeap)));
            this->srvDesSize = base->device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        }
        if (dsvCap > 0)
        {
            D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
            dsvHeapDesc.NumDescriptors = dsvCap;
            dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
            dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
            SK_CHECK(base->device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&this->dsvHeap)));
            this->dsvDesSize = base->device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
        }
    }

    CD3DX12_CPU_DESCRIPTOR_HANDLE GetSRV(int offset)
    {
        SK_N_NULL(offset < (int)srvCap);
        CD3DX12_CPU_DESCRIPTOR_HANDLE handle{srvHeap->GetCPUDescriptorHandleForHeapStart(), offset, srvDesSize};
        return handle;
    }
    CD3DX12_GPU_DESCRIPTOR_HANDLE GetSRV_G(int offset)
    {
        SK_N_NULL(offset < (int)srvCap);
        CD3DX12_GPU_DESCRIPTOR_HANDLE handle{srvHeap->GetGPUDescriptorHandleForHeapStart(), offset, srvDesSize};
        return handle;
    }
    ID3D12DescriptorHeap *GetHeapSRV()
    {
        return srvHeap.Get();
    }
    ID3D12DescriptorHeap *GetHeapRTV()
    {
        return rtvHeap.Get();
    }
    ID3D12DescriptorHeap *GetHeapDSV()
    {
        return dsvHeap.Get();
    }
    CD3DX12_CPU_DESCRIPTOR_HANDLE GetRTV(int offset)
    {
        SK_N_NULL(offset < (int)rtvCap);
        CD3DX12_CPU_DESCRIPTOR_HANDLE handle{rtvHeap->GetCPUDescriptorHandleForHeapStart(), offset, rtvDesSize};
        return handle;
    }
    CD3DX12_GPU_DESCRIPTOR_HANDLE GetRTV_G(int offset)
    {
        SK_N_NULL(offset < (int)rtvCap);
        CD3DX12_GPU_DESCRIPTOR_HANDLE handle{rtvHeap->GetGPUDescriptorHandleForHeapStart(), offset, rtvDesSize};
        return handle;
    }
};