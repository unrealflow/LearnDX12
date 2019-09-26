#pragma once
#include "SkBase.h"
#include "SkTarget.h"
#include "SkMesh.h"

class SkPass
{
private:
    SkBase *base = nullptr;
    std::vector<ID3D12DescriptorHeap *> pHeaps;
    std::vector<SkMesh *> meshes;
    std::unordered_map<uint32_t, D3D12_GPU_DESCRIPTOR_HANDLE> tableDescs;
    std::unordered_map<uint32_t, D3D12_GPU_VIRTUAL_ADDRESS> constDescs;
    SkTarget *RT;
public:
    SkPass() {}
    ~SkPass() {}
    ComPtr<ID3D12PipelineState> pipeline;
    ComPtr<ID3D12RootSignature> root;

    void Init(SkBase *initBase)
    {
        base = initBase;
        pHeaps.clear();
        meshes.clear();
        tableDescs.clear();
        constDescs.clear();
    }
    void AddMesh(SkMesh *mesh)
    {
        if (mesh == nullptr)
            return;
        meshes.emplace_back(mesh);
    }
    void AddDesc(uint32_t index, ID3D12DescriptorHeap *heap, int offset = 0)
    {
        pHeaps.push_back(heap);
        if (0 == offset)
        {
            tableDescs[index] = heap->GetGPUDescriptorHandleForHeapStart();
        }
        else
        {
            CD3DX12_GPU_DESCRIPTOR_HANDLE handle{heap->GetGPUDescriptorHandleForHeapStart(), offset, base->heap->srvDesSize};
            tableDescs[index] = handle;
        }
    }
    void AddDesc(uint32_t index, D3D12_GPU_VIRTUAL_ADDRESS address)
    {
        constDescs[index] = address;
    }

    void CmdSet(ComPtr<ID3D12GraphicsCommandList> cmd, uint32_t index)
    {
        CD3DX12_VIEWPORT m_viewport{0.0f, 0.0f, static_cast<float>(base->width), static_cast<float>(base->height)};
        CD3DX12_RECT m_scissorRect{0, 0, static_cast<LONG>(base->width), static_cast<LONG>(base->height)};

        cmd->SetPipelineState(pipeline.Get());
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
        cmd->RSSetViewports(1, &m_viewport);
        cmd->RSSetScissorRects(1, &m_scissorRect);

        // Indicate that the back buffer will be used as a render target.
        RT->PreBarrier(cmd, index);
        auto handles = RT->Get(index);

        if (0 == meshes.size())
        {
            cmd->OMSetRenderTargets(RT->Size(), handles.data(), FALSE, nullptr);
            for (uint32_t t = 0; t < RT->Size(); t++)
            {
                cmd->ClearRenderTargetView(handles[t], base->clearColor, 0, nullptr);
            }
            // Record commands.
            cmd->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
            cmd->DrawInstanced(3, 1, 0, 0);
        }
        else
        {
            cmd->OMSetRenderTargets(RT->Size(), handles.data(), FALSE, &base->heap->GetHeapDSV()->GetCPUDescriptorHandleForHeapStart());
            for (uint32_t t = 0; t < RT->Size(); t++)
            {
                cmd->ClearRenderTargetView(handles[t], base->clearColor, 0, nullptr);
            }
            // Record commands.
            cmd->ClearDepthStencilView(base->heap->GetHeapDSV()->GetCPUDescriptorHandleForHeapStart(), D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
            for (uint32_t mc = 0; mc < meshes.size(); mc++)
            {
                meshes[mc]->Draw(cmd.Get());
            }
        }
        RT->AftBarrier(cmd, index);
    }
    void CreateRoot(std::vector<CD3DX12_ROOT_PARAMETER1> &rootParameters, SkTarget *rt) //,std::vector<D3D12_INPUT_ELEMENT_DESC> &inputDescs)
    {
        this->RT = rt;
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
    // AddMesh() should be invoked before this if needed
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
                         nullptr, &base->include, "VSMain", "vs_5_1",
                         compileFlags, 0, &vertexShader, &errorMessage),
                     errorMessage);

        SK_CHECK_MSG(D3DCompileFromFile(
                         GetAssetFullPath(Shader).c_str(),
                         nullptr, &base->include, "PSMain", "ps_5_1",
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
        // rs.DepthClipEnable=true;
        // rs.CullMode = D3D12_CULL_MODE_NONE;
        psoDesc.RasterizerState = rs;
        psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
        psoDesc.DepthStencilState.DepthEnable = FALSE;
        psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC::D3D12_COMPARISON_FUNC_LESS;
        psoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
        psoDesc.DepthStencilState.StencilEnable = FALSE;
        psoDesc.SampleMask = UINT_MAX;
        psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        psoDesc.NumRenderTargets = RT->Size();
        if (meshes.size() > 0)
        {
            psoDesc.DepthStencilState.DepthEnable = TRUE;
            psoDesc.DSVFormat = base->depthFormat;
        }
        for (uint32_t i = 0; i < RT->Size(); i++)
        {
            psoDesc.RTVFormats[i] = RT->GetFormat(i);
        }
        psoDesc.SampleDesc.Count = 1;
        SK_CHECK(base->device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&this->pipeline)));
    }
};
