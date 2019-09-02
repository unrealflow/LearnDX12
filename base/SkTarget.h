#pragma once
#include "SkBase.h"

class SkTarget
{
private:
    /* data */
public:
    virtual uint32_t Size() = 0;
    virtual DXGI_FORMAT GetFormat(uint32_t p) = 0;
    virtual std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> Get(int index) = 0;
    virtual void PreBarrier(ComPtr<ID3D12GraphicsCommandList> cmd, uint32_t index) = 0;
    virtual void AftBarrier(ComPtr<ID3D12GraphicsCommandList> cmd, uint32_t index) = 0;
};

class SkDefaultRT : public SkTarget
{
private:
    SkBase *base;
    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle;

public:
    void Init(SkBase *initBase)
    {
        base = initBase;
    }
    virtual uint32_t Size() override
    {
        return 1;
    }
    virtual DXGI_FORMAT GetFormat(uint32_t p)
    {
        return base->format;
    }
    virtual std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> Get(int index) override
    {
        return {CD3DX12_CPU_DESCRIPTOR_HANDLE{
            base->rtvHeap->GetCPUDescriptorHandleForHeapStart(),
            index,
            base->rtvDesSize}};
    }
    virtual void PreBarrier(ComPtr<ID3D12GraphicsCommandList> cmd, uint32_t index)
    {
        cmd->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(base->renderTargets[index].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));
    }
    virtual void AftBarrier(ComPtr<ID3D12GraphicsCommandList> cmd, uint32_t index)
    {
        cmd->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(base->renderTargets[index].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));
    }
};

class SkImageRT : public SkTarget
{
private:
    SkBase *base;
    D3D12_CPU_DESCRIPTOR_HANDLE srvHandle;
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle;

public:
    ComPtr<ID3D12Resource> texture;
    DXGI_FORMAT format;
    void Init(SkBase *initBase, DXGI_FORMAT format = DXGI_FORMAT_R32G32B32A32_FLOAT)
    {
        base = initBase;
        this->format = format;
    }
    void CreateResource()
    {
        D3D12_RESOURCE_DESC textureDesc = {};
        textureDesc.MipLevels = 1;
        textureDesc.Format = this->format;
        textureDesc.Width = base->width;
        textureDesc.Height = base->height;
        textureDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
        textureDesc.DepthOrArraySize = 1;
        textureDesc.SampleDesc.Count = 1;
        textureDesc.SampleDesc.Quality = 0;
        textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;

        SK_CHECK(base->device->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
            D3D12_HEAP_FLAG_NONE,
            &textureDesc,
            D3D12_RESOURCE_STATE_COMMON,
            nullptr,
            IID_PPV_ARGS(&texture)));
    }
    void CreateView(D3D12_CPU_DESCRIPTOR_HANDLE srvHandle, D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle)
    {
        this->srvHandle = srvHandle;
        this->rtvHandle = rtvHandle;

        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        srvDesc.Format = this->format;
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MipLevels = 1;
        base->device->CreateShaderResourceView(texture.Get(), &srvDesc, srvHandle);

        D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
        rtvDesc.Format = this->format;
        rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
        base->device->CreateRenderTargetView(texture.Get(), &rtvDesc, rtvHandle);
    }
    virtual uint32_t Size() override
    {
        return 1;
    }
    virtual std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> Get(int index) override
    {
        // return {CD3DX12_CPU_DESCRIPTOR_HANDLE{
        //             base->rtvHeap->GetCPUDescriptorHandleForHeapStart(),
        //             index,
        //             base->rtvDesSize},
        //         this->rtvHandle};
        return {this->rtvHandle};
    }
    virtual void PreBarrier(ComPtr<ID3D12GraphicsCommandList> cmd, uint32_t index)
    {
        // cmd->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(base->renderTargets[index].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));
        cmd->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(this->texture.Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_RENDER_TARGET));
    }
    virtual void AftBarrier(ComPtr<ID3D12GraphicsCommandList> cmd, uint32_t index)
    {
        // cmd->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(base->renderTargets[index].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));
        cmd->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(this->texture.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COMMON));
    }
    virtual DXGI_FORMAT GetFormat(uint32_t p)
    {   
        // if(p==0) return base->format;
        return this->format;
    }
};