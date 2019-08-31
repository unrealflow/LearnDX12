#pragma once
#include "SkBase.h"

class SkPass
{
private:
    SkBase *base = nullptr;
    std::vector<ID3D12DescriptorHeap *> pHeaps;
    std::unordered_map<uint32_t, D3D12_GPU_DESCRIPTOR_HANDLE> tableDescs;
    std::unordered_map<uint32_t, D3D12_GPU_VIRTUAL_ADDRESS> constDescs;

public:
    SkPass() {}
    ~SkPass() {}
    ComPtr<ID3D12PipelineState> pipeline;
    ComPtr<ID3D12RootSignature> root;

    void Init(SkBase *initBase)
    {
        base = initBase;
        pHeaps.clear();
        tableDescs.clear();
        constDescs.clear();
    }
    void Add(uint32_t index, ID3D12DescriptorHeap *heap, int offset = 0)
    {
        pHeaps.push_back(heap);
        if (0 == offset)
        {
            tableDescs[index] = heap->GetGPUDescriptorHandleForHeapStart();
        }
        else
        {
            CD3DX12_GPU_DESCRIPTOR_HANDLE handle{heap->GetGPUDescriptorHandleForHeapStart(), offset, base->srvDesSize};
            tableDescs[index] = handle;
        }
    }
    void Add(uint32_t index, D3D12_GPU_VIRTUAL_ADDRESS address)
    {
        constDescs[index] = address;
    }
    void CmdSet(ComPtr<ID3D12GraphicsCommandList> cmd, bool reset = true)
    {
        if (reset)
        {
            cmd->Reset(base->cmdPool.Get(), pipeline.Get());
        }
        else
        {
            cmd->SetPipelineState(pipeline.Get());
        }
        cmd->SetGraphicsRootSignature(root.Get());
        cmd->SetDescriptorHeaps(static_cast<uint32_t>(pHeaps.size()), pHeaps.data());
        for (auto desc = tableDescs.begin(); desc != tableDescs.end(); desc++)
        {
            cmd->SetGraphicsRootDescriptorTable(desc->first, desc->second);
        }
        for (auto desc = constDescs.begin(); desc != constDescs.end(); desc++)
        {
            cmd->SetGraphicsRootConstantBufferView(desc->first, desc->second);
        }
    }
    void CreateRoot(std::vector<CD3DX12_ROOT_PARAMETER1> &rootParameters) //,std::vector<D3D12_INPUT_ELEMENT_DESC> &inputDescs)
    {

        // Create an empty root signature.
        D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};

        // This is the highest version the sample supports. If CheckFeatureSupport succeeds, the HighestVersion returned will not be greater than this.
        featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;

        if (FAILED(base->device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData))))
        {
            featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
        }

        CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
        rootSignatureDesc.Init_1_1((uint32_t)rootParameters.size(), rootParameters.data(), 1,
                                   &InitSampler(), D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

        ComPtr<ID3DBlob> signature;
        ComPtr<ID3DBlob> error;
        SK_CHECK_MSG(D3DX12SerializeVersionedRootSignature(&rootSignatureDesc,
                                                           featureData.HighestVersion,
                                                           &signature, &error),
                     error);
        SK_CHECK(base->device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&this->root)));
    }
    //"VSMain","PSMain","vs_5_1"
    void CreatePipeline(std::vector<D3D12_INPUT_ELEMENT_DESC> &inputDescs, std::wstring Shader)
    {
        // Create the pipeline state, which includes compiling and loading shaders.

        ComPtr<ID3DBlob> vertexShader;
        ComPtr<ID3DBlob> pixelShader;
        ComPtr<ID3DBlob> errorMessage;
        // #if defined(_DEBUG)
        // Enable better shader debugging with the graphics debugging tools.
        UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
        // #else
        // UINT compileFlags = 0;
        // #endif

        SK_CHECK_MSG(D3DCompileFromFile(
                         GetAssetFullPath(Shader).c_str(),
                         nullptr, nullptr, "VSMain", "vs_5_1",
                         compileFlags, 0, &vertexShader, &errorMessage),
                     errorMessage);

        SK_CHECK_MSG(D3DCompileFromFile(
                         GetAssetFullPath(Shader).c_str(),
                         nullptr, nullptr, "PSMain", "ps_5_1",
                         compileFlags, 0, &pixelShader, &errorMessage),
                     errorMessage);

        // Describe and create the graphics pipeline state object (PSO).
        D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
        psoDesc.InputLayout = {inputDescs.data(), static_cast<uint32_t>(inputDescs.size())};
        psoDesc.pRootSignature = this->root.Get();
        psoDesc.VS = CD3DX12_SHADER_BYTECODE(vertexShader.Get());
        psoDesc.PS = CD3DX12_SHADER_BYTECODE(pixelShader.Get());
        auto rs = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
        // rs.FillMode=D3D12_FILL_MODE_WIREFRAME;
        rs.CullMode = D3D12_CULL_MODE_NONE;
        psoDesc.RasterizerState = rs;
        psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
        psoDesc.DepthStencilState.DepthEnable = TRUE;
        psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC::D3D12_COMPARISON_FUNC_LESS;
        psoDesc.DepthStencilState.StencilEnable = FALSE;
        psoDesc.SampleMask = UINT_MAX;
        psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        psoDesc.NumRenderTargets = 1;
        psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
        psoDesc.SampleDesc.Count = 1;
        SK_CHECK(base->device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&this->pipeline)));
    }
};
