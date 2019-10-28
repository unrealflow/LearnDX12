#pragma once
#include "SkBase.h"
#include "SkHeap.h"
class ISkTarget
{
private:
    /* data */
public:
    virtual uint32_t Size() = 0;
    virtual DXGI_FORMAT GetFormat(uint32_t p) = 0;
    virtual std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> Get(int index) = 0;
    virtual void PreBarrier(ComPtr<ID3D12GraphicsCommandList> cmd, uint32_t index) = 0;
    virtual void AftBarrier(ComPtr<ID3D12GraphicsCommandList> cmd, uint32_t index) = 0;
    virtual bool ShouldClear() { return true; }
};
//Use SwapChain's handle
class SkDefaultRT : public ISkTarget
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
        return {base->heap->GetRTV(index)};
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

class SkImageRT : public ISkTarget
{
private:
    SkBase *base;
    D3D12_CPU_DESCRIPTOR_HANDLE srvHandle;
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle;
    bool shouldClear=true;
    void CreateResource()
    {
        D3D12_RESOURCE_DESC textureDesc = {};
        textureDesc.MipLevels = 1;
        textureDesc.Format = this->format;
        textureDesc.Width = base->width;
        textureDesc.Height = base->height;
        textureDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
        textureDesc.DepthOrArraySize = 1;
        textureDesc.SampleDesc.Count = 1;
        textureDesc.SampleDesc.Quality = 0;
        textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;

        CD3DX12_CLEAR_VALUE clearValue{this->format, base->clearColor};
        SK_CHECK(base->device->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
            D3D12_HEAP_FLAG_NONE,
            &textureDesc,
            D3D12_RESOURCE_STATE_COMMON,
            &clearValue,
            IID_PPV_ARGS(&texture)));
    }
    void CreateView(D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle, D3D12_CPU_DESCRIPTOR_HANDLE srvHandle)
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

public:
    ComPtr<ID3D12Resource> texture;
    DXGI_FORMAT format;
    void Init(SkBase *initBase, DXGI_FORMAT format = DXGI_FORMAT_R32G32B32A32_FLOAT)
    {
        base = initBase;
        this->format = format;
    }
    //Need 1 handle
    void Setup(SkHeap *heap, int rtvBegin, int srvBegin,bool shouldClear=true)
    {
        this->shouldClear=shouldClear;
        CreateResource();
        CreateView(heap->GetRTV(rtvBegin), heap->GetSRV(srvBegin));
    }
    virtual bool ShouldClear() override
    {
        return this->shouldClear;
    }
    virtual uint32_t Size() override
    {
        return 1;
    }
    virtual std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> Get(int index) override
    {
        return {this->rtvHandle};
    }
    virtual void PreBarrier(ComPtr<ID3D12GraphicsCommandList> cmd, uint32_t index)
    {
        cmd->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(this->texture.Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_RENDER_TARGET));
    }
    virtual void AftBarrier(ComPtr<ID3D12GraphicsCommandList> cmd, uint32_t index)
    {
        cmd->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(this->texture.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COMMON));
    }
    virtual DXGI_FORMAT GetFormat(uint32_t p)
    {
        return this->format;
    }
};
//Positon 
//Normal 
//Albedo
class SkGBufferRT : public ISkTarget
{
private:
    SkBase *base = nullptr;
    SkHeap *heap = nullptr;
    int rtvBegin = 0;
    int srvBegin = 0;
    void CreateResource(ComPtr<ID3D12Resource> &image)
    {
        D3D12_RESOURCE_DESC imageDesc = {};
        imageDesc.MipLevels = 1;
        imageDesc.Format = this->format;
        imageDesc.Width = base->width;
        imageDesc.Height = base->height;
        imageDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
        imageDesc.DepthOrArraySize = 1;
        imageDesc.SampleDesc.Count = 1;
        imageDesc.SampleDesc.Quality = 0;
        imageDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;

        CD3DX12_CLEAR_VALUE clearValue{this->format, base->clearColor};
        SK_CHECK(base->device->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
            D3D12_HEAP_FLAG_NONE,
            &imageDesc,
            D3D12_RESOURCE_STATE_COMMON,
            &clearValue,
            IID_PPV_ARGS(&image)));
    }
    void CreateView(ComPtr<ID3D12Resource> &image, int offset)
    {
        {
            D3D12_CPU_DESCRIPTOR_HANDLE srvHandle = this->heap->GetSRV(srvBegin + offset);

            D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
            srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
            srvDesc.Format = this->format;
            srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
            srvDesc.Texture2D.MipLevels = 1;
            base->device->CreateShaderResourceView(image.Get(), &srvDesc, srvHandle);
        }

        {
            D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = this->heap->GetRTV(rtvBegin + offset);
            D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
            rtvDesc.Format = this->format;
            rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
            base->device->CreateRenderTargetView(image.Get(), &rtvDesc, rtvHandle);
        }
    }

public:
    ComPtr<ID3D12Resource> position;
    ComPtr<ID3D12Resource> normal;
    ComPtr<ID3D12Resource> albedo;
    DXGI_FORMAT format;
    void Init(SkBase *initBase, DXGI_FORMAT format = DXGI_FORMAT_R32G32B32A32_FLOAT)
    {
        base = initBase;
        this->format = format;
    }
    //Needs 3 handles
    void Setup(SkHeap *heap, int rtvBegin, int srvBegin)
    {
        this->heap = heap;
        this->rtvBegin = rtvBegin;
        this->srvBegin = srvBegin;
        CreateResource(position);
        CreateResource(normal);
        CreateResource(albedo);
        CreateView(position, 0);
        CreateView(normal, 1);
        CreateView(albedo, 2);
    }
    virtual uint32_t Size() override
    {
        return 3;
    }
    virtual std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> Get(int index) override
    {
        return {heap->GetRTV(rtvBegin + 0), heap->GetRTV(rtvBegin + 1), heap->GetRTV(rtvBegin + 2)};
    }
    virtual void PreBarrier(ComPtr<ID3D12GraphicsCommandList> cmd, uint32_t index)
    {
        cmd->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(this->position.Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_RENDER_TARGET));
        cmd->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(this->normal.Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_RENDER_TARGET));
        cmd->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(this->albedo.Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_RENDER_TARGET));
    }
    virtual void AftBarrier(ComPtr<ID3D12GraphicsCommandList> cmd, uint32_t index)
    {
        cmd->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(this->position.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COMMON));
        cmd->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(this->normal.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COMMON));
        cmd->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(this->albedo.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COMMON));
    }
    virtual DXGI_FORMAT GetFormat(uint32_t p)
    {
        // if(p==0) return base->format;
        return this->format;
    }
};