﻿#pragma once
#include "SkBase.h"

class SkHeap
{
private:
    SkBase *base;
    uint32_t rtvCap;
    uint32_t srvCap;
    ComPtr<ID3D12DescriptorHeap> rtvHeap;
    ComPtr<ID3D12DescriptorHeap> srvHeap;
    ComPtr<ID3D12DescriptorHeap> dsvHeap;
    uint32_t rtvDesSize;
    uint32_t srvDesSize;
    uint32_t dsvDesSize;
    uint32_t rtvCount;
    uint32_t srvCount;

public:
    void Init(SkBase *initBase,
              uint32_t rtvCap,
              uint32_t srvCap)
    {
        base = initBase;
        this->rtvCap = rtvCap;
        this->srvCap = srvCap;
        this->rtvCount = 0;
        this->srvCount = 0;
        D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
        rtvHeapDesc.NumDescriptors = rtvCap;
        rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        SK_CHECK(base->device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&this->rtvHeap)));

        D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
        srvHeapDesc.NumDescriptors = srvCap;
        srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        SK_CHECK(base->device->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&this->srvHeap)));

        D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
        srvHeapDesc.NumDescriptors = 2;
        srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
        srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        SK_CHECK(base->device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&this->dsvHeap)));

        this->rtvDesSize = base->device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
        this->srvDesSize = base->device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        this->dsvDesSize = base->device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
    }
    uint32_t AllocSRV(uint32_t _size = 1)
    {
        srvCount += _size;
        SK_N_NULL(srvCount <= srvCap);
        return srvCount - _size;
    }
    uint32_t AllocRTV(uint32_t _size = 1)
    {
        rtvCount += _size;
        SK_N_NULL(rtvCount <= rtvCap);
        return rtvCount - _size;
    }
    CD3DX12_CPU_DESCRIPTOR_HANDLE GetSRV(int offset)
    {
        CD3DX12_CPU_DESCRIPTOR_HANDLE handle{srvHeap->GetCPUDescriptorHandleForHeapStart(), offset, srvDesSize};
        return handle;
    }
    CD3DX12_GPU_DESCRIPTOR_HANDLE GetSRV_G(int offset)
    {
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
    CD3DX12_CPU_DESCRIPTOR_HANDLE GetRTV(int offset)
    {
        CD3DX12_CPU_DESCRIPTOR_HANDLE handle{rtvHeap->GetCPUDescriptorHandleForHeapStart(), offset, rtvDesSize};
        return handle;
    }
    CD3DX12_GPU_DESCRIPTOR_HANDLE GetRTV_G(int offset)
    {
        CD3DX12_GPU_DESCRIPTOR_HANDLE handle{rtvHeap->GetGPUDescriptorHandleForHeapStart(), offset, rtvDesSize};
        return handle;
    }
};