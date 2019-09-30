#pragma once
#include "SkTarget.h"
#include "SkHeap.h"
class SkComputer : public ISkPass
{
private:
    SkBase *base;
    ComPtr<ID3D12RootSignature> root;
    ComPtr<ID3D12PipelineState> pipeline;
    std::unordered_map<uint32_t, D3D12_GPU_VIRTUAL_ADDRESS> constDescs;
    SkHeap heap;
    ID3D12Resource *input = nullptr;
    ID3D12Resource *output = nullptr;

public:
    ComPtr<ID3D12Resource> texture;
    void Init(SkBase *initBase, uint32_t size = 2)
    {
        base = initBase;
        constDescs.clear();
        heap.Init(base, 0, size, 0);
    }
    void AddDesc(uint32_t index, D3D12_GPU_VIRTUAL_ADDRESS address)
    {
        constDescs[index] = address;
    }
    void UseDefaultTexture(DXGI_FORMAT format)
    {
        D3D12_RESOURCE_DESC textureDesc = {};
        textureDesc.MipLevels = 1;
        textureDesc.Format = format;
        textureDesc.Width = base->width;
        textureDesc.Height = base->height;
        textureDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
        textureDesc.DepthOrArraySize = 1;
        textureDesc.SampleDesc.Count = 1;
        textureDesc.SampleDesc.Quality = 0;
        textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;

        CD3DX12_CLEAR_VALUE clearValue{format, base->clearColor};
        SK_CHECK(base->device->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
            D3D12_HEAP_FLAG_NONE,
            &textureDesc,
            D3D12_RESOURCE_STATE_COMMON,
            &clearValue,
            IID_PPV_ARGS(&texture)));

        CreateOutputView(this->texture.Get(), format);
    }
    void CreateRoot(std::vector<CD3DX12_ROOT_PARAMETER1> *rootParameters)
    {
        CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
        CD3DX12_DESCRIPTOR_RANGE1 ranges[2];
        ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_VOLATILE);
        ranges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 1, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_VOLATILE);
        std::vector<CD3DX12_ROOT_PARAMETER1> _rootParameters{1};
        _rootParameters[0].InitAsDescriptorTable(2, &ranges[0]);

        if (rootParameters != nullptr)
        {
            rootSignatureDesc.Init_1_1((uint32_t)rootParameters->size(),
                                       rootParameters->data(), 1,
                                       &base->sampler, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
        }
        else
        {
            rootSignatureDesc.Init_1_1((uint32_t)_rootParameters.size(),
                                       _rootParameters.data(), 1,
                                       &base->sampler, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
        }

        ComPtr<ID3DBlob> signature;
        ComPtr<ID3DBlob> error;
        SK_CHECK_MSG(D3DX12SerializeVersionedRootSignature(&rootSignatureDesc,
                                                           base->featureData.HighestVersion,
                                                           &signature, &error),
                     error);
        SK_CHECK(base->device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&this->root)));
    }
    void CreatePipeline(std::wstring shader)
    {
        ComPtr<ID3DBlob> computeShader;
        ComPtr<ID3DBlob> errorMessage;

        UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;

        SK_CHECK_MSG(D3DCompileFromFile(
                         GetAssetFullPath(shader).c_str(),
                         nullptr, &base->include, "CSMain", "cs_5_1",
                         compileFlags, 0, &computeShader, &errorMessage),
                     errorMessage);
        D3D12_COMPUTE_PIPELINE_STATE_DESC psoDesc = {};
        psoDesc.pRootSignature = this->root.Get();
        psoDesc.CS = CD3DX12_SHADER_BYTECODE(computeShader.Get());
        base->device->CreateComputePipelineState(&psoDesc, IID_PPV_ARGS(&this->pipeline));
    }
    void CreateInputView(ID3D12Resource *inputImage, DXGI_FORMAT format, int offset = 0)
    {
        this->input = inputImage;
        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        srvDesc.Format = format;
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MipLevels = 1;
        base->device->CreateShaderResourceView(inputImage, &srvDesc, this->heap.GetSRV(offset));
    }
    void CreateOutputView(ID3D12Resource *outputImage, DXGI_FORMAT format, int offset = 1)
    {
        this->output = outputImage;
        D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
        uavDesc.Format = format;
        uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
        uavDesc.Texture2D.MipSlice = 0;

        base->device->CreateUnorderedAccessView(outputImage, nullptr, &uavDesc, this->heap.GetSRV(offset));
    }
    void CmdSet(ComPtr<ID3D12GraphicsCommandList> cmd, uint32_t index)
    {
        cmd->SetPipelineState(this->pipeline.Get());
        cmd->SetComputeRootSignature(this->root.Get());
        std::vector<ID3D12DescriptorHeap *> pHeaps{this->heap.GetHeapSRV()};
        cmd->SetDescriptorHeaps(static_cast<uint32_t>(pHeaps.size()), pHeaps.data());
        cmd->SetComputeRootDescriptorTable(0, this->heap.GetSRV_G(0));
        for (auto desc = constDescs.begin(); desc != constDescs.end(); desc++)
        {
            cmd->SetComputeRootConstantBufferView(desc->first, desc->second);
        }
        cmd->Dispatch(base->width, base->height, 1);
        cmd->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(this->output, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_SOURCE));
        cmd->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(this->input, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST));
        cmd->CopyResource(this->input, this->output);
        cmd->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(this->output, D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_COMMON));
        cmd->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(this->input, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_COMMON));
    }
};