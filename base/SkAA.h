#pragma once
#include "SkAgent.h"
#include "SkTarget.h"

//Record the image、postion and normal of previous frame
class SkAA : public ISkPass
{
private:
    SkBase *base = nullptr;
    SkHeap *heap = nullptr;
    SkGBufferRT *gBuffer = nullptr;
    SkImageRT *preImage = nullptr;
    int srvBegin = 0;
    void CreateResource(ComPtr<ID3D12Resource> &image, DXGI_FORMAT format)
    {
        D3D12_RESOURCE_DESC imageDesc = {};
        imageDesc.MipLevels = 1;
        imageDesc.Format = format;
        imageDesc.Width = base->width;
        imageDesc.Height = base->height;
        imageDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
        imageDesc.DepthOrArraySize = 1;
        imageDesc.SampleDesc.Count = 1;
        imageDesc.SampleDesc.Quality = 0;
        imageDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;

        CD3DX12_CLEAR_VALUE clearValue{format, base->clearColor};
        SK_CHECK(base->device->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
            D3D12_HEAP_FLAG_NONE,
            &imageDesc,
            D3D12_RESOURCE_STATE_COMMON,
            &clearValue,
            IID_PPV_ARGS(&image)));
    }
    void CreateView(ComPtr<ID3D12Resource> &image, DXGI_FORMAT format, int offset)
    {
        {
            D3D12_CPU_DESCRIPTOR_HANDLE srvHandle = this->heap->GetSRV(srvBegin + offset);

            D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
            srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
            srvDesc.Format = format;
            srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
            srvDesc.Texture2D.MipLevels = 1;
            base->device->CreateShaderResourceView(image.Get(), &srvDesc, srvHandle);
        }
    }

public:
    ComPtr<ID3D12Resource> image;
    ComPtr<ID3D12Resource> position;
    ComPtr<ID3D12Resource> normal;

    void Init(SkBase *initBase, SkGBufferRT *gBuffer, SkImageRT *preImage)
    {
        base=initBase;
        this->gBuffer = gBuffer;
        this->preImage = preImage;
    }
    //Need 3 srv handles
    void Setup(SkHeap *heap, int srvBegin)
    {
        this->heap = heap;
        this->srvBegin = srvBegin;
        CreateResource(image, preImage->GetFormat(0));
        CreateResource(position, gBuffer->GetFormat(0));
        CreateResource(normal, gBuffer->GetFormat(1));
        CreateView(image, preImage->GetFormat(0), 0);
        CreateView(position, gBuffer->GetFormat(0), 1);
        CreateView(normal, gBuffer->GetFormat(1), 2);
    }
    void CmdSet(ComPtr<ID3D12GraphicsCommandList> cmd, uint32_t index)
    {
       static std::vector<D3D12_RESOURCE_BARRIER> preBarriers =
            {
                CD3DX12_RESOURCE_BARRIER::Transition(preImage->texture.Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_SOURCE),
                CD3DX12_RESOURCE_BARRIER::Transition(gBuffer->position.Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_SOURCE),
                CD3DX12_RESOURCE_BARRIER::Transition(gBuffer->normal.Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_SOURCE),

                CD3DX12_RESOURCE_BARRIER::Transition(this->image.Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST),
                CD3DX12_RESOURCE_BARRIER::Transition(this->position.Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST),
                CD3DX12_RESOURCE_BARRIER::Transition(this->normal.Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST),
            };
        static std::vector<D3D12_RESOURCE_BARRIER> aftBarriers =
            {
                CD3DX12_RESOURCE_BARRIER::Transition(preImage->texture.Get(), D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_COMMON),
                CD3DX12_RESOURCE_BARRIER::Transition(gBuffer->position.Get(), D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_COMMON),
                CD3DX12_RESOURCE_BARRIER::Transition(gBuffer->normal.Get(), D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_COMMON),

                CD3DX12_RESOURCE_BARRIER::Transition(this->image.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_COMMON),
                CD3DX12_RESOURCE_BARRIER::Transition(this->position.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_COMMON),
                CD3DX12_RESOURCE_BARRIER::Transition(this->normal.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_COMMON),
            };
        cmd->ResourceBarrier((UINT)preBarriers.size(), preBarriers.data());
        cmd->CopyResource(this->image.Get(), preImage->texture.Get());
        cmd->CopyResource(this->position.Get(), gBuffer->position.Get());
        cmd->CopyResource(this->normal.Get(), gBuffer->normal.Get());
        cmd->ResourceBarrier((UINT)aftBarriers.size(), aftBarriers.data());
    }
};