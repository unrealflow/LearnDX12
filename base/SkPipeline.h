#pragma once
#include "SkBase.h"

class SkPipeline
{
private:
    SkBase *base;
    void InitSwapChain()
    {
        uint32_t dxgiFactoryFlags = 0;
        // #if defined(_DEBUG)
        // Enable the debug layer (requires the Graphics Tools "optional feature").
        // NOTE: Enabling the debug layer after device creation will invalidate the active device.
        {
            ComPtr<ID3D12Debug> debugController;
            if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
            {
                debugController->EnableDebugLayer();

                // Enable additional debug layers.
                dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
            }
        }
        // #endif
        ComPtr<IDXGIFactory4> factory;
        SK_CHECK(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&factory)));

        // if (base->useWarpDevice)
        // {
        //     ComPtr<IDXGIAdapter> warpAdapter;
        //     SK_CHECK(factory->EnumWarpAdapter(IID_PPV_ARGS(&warpAdapter)));

        //     SK_CHECK(D3D12CreateDevice(
        //         warpAdapter.Get(),
        //         D3D_FEATURE_LEVEL_11_0,
        //         IID_PPV_ARGS(&base->device)));
        // }
        // else
        {
            ComPtr<IDXGIAdapter1> hardwareAdapter;
            GetHardwareAdapter(factory.Get(), &hardwareAdapter);

            SK_CHECK(D3D12CreateDevice(
                hardwareAdapter.Get(),
                D3D_FEATURE_LEVEL_11_0,
                IID_PPV_ARGS(&base->device)));
        }
        D3D12_COMMAND_QUEUE_DESC queueDesc = {};
        queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

        SK_CHECK(base->device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&base->cmdQueue)));

        // Describe and create the swap chain.
        DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
        swapChainDesc.BufferCount = base->imageCount;
        swapChainDesc.Width = base->width;
        swapChainDesc.Height = base->height;
        swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        swapChainDesc.SampleDesc.Count = 1;

        ComPtr<IDXGISwapChain1> swapChain;
        SK_CHECK(factory->CreateSwapChainForHwnd(
            base->cmdQueue.Get(), // Swap chain needs the queue so that it can force a flush on it.
            base->hwnd,
            &swapChainDesc,
            nullptr,
            nullptr,
            &swapChain));
        // This sample does not support fullscreen transitions.
        SK_CHECK(factory->MakeWindowAssociation(base->GetHwnd(), DXGI_MWA_NO_ALT_ENTER));

        SK_CHECK(swapChain.As(&base->swapChain));
        base->imageIndex = base->swapChain->GetCurrentBackBufferIndex();

        // Create descriptor heaps.
        {
            // Describe and create a render target view (RTV) descriptor heap.
            D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
            rtvHeapDesc.NumDescriptors = base->imageCount;
            rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
            rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
            SK_CHECK(base->device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&base->rtvHeap)));

            D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
            srvHeapDesc.NumDescriptors = 1;
            srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
            srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
            SK_CHECK(base->device->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&base->srvHeap)));

            // D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
            // srvHeapDesc.NumDescriptors = 1;
            // srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
            // srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
            // SK_CHECK(base->device->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&base->srvHeap)));

            base->rtvDesSize = base->device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
            base->srvDesSize = base->device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
            // base->dsvDesSize = base->device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
            fprintf(stderr,"base->rtvDesSize:%d...\n",base->rtvDesSize);
            fprintf(stderr,"base->srvDesSize:%d...\n",base->srvDesSize);
            // fprintf(stderr,"base->dsvDesSize:%d...\n",base->dsvDesSize);
            
        }

        // Create frame resources.
        {
            CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(base->rtvHeap->GetCPUDescriptorHandleForHeapStart());

            base->renderTargets.resize(base->imageCount);
            // Create a RTV for each frame.
            for (UINT n = 0; n < base->imageCount; n++)
            {
                SK_CHECK(base->swapChain->GetBuffer(n, IID_PPV_ARGS(&base->renderTargets[n])));
                base->device->CreateRenderTargetView(base->renderTargets[n].Get(), nullptr, rtvHandle);
                rtvHandle.Offset(1, base->rtvDesSize);
            }
        }

        SK_CHECK(base->device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&base->cmdPool)));
    }

    void GetHardwareAdapter(IDXGIFactory4 *pFactory, IDXGIAdapter1 **ppAdapter)
    {
        ComPtr<IDXGIAdapter1> adapter;
        *ppAdapter = nullptr;

        for (UINT adapterIndex = 0; DXGI_ERROR_NOT_FOUND != pFactory->EnumAdapters1(adapterIndex, &adapter); ++adapterIndex)
        {
            DXGI_ADAPTER_DESC1 desc;
            adapter->GetDesc1(&desc);

            if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
            {
                // Don't select the Basic Render Driver adapter.
                continue;
            }

            // Check to see if the adapter supports Direct3D 12, but don't create the
            // actual device yet.
            if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
            {
                break;
            }
        }

        *ppAdapter = adapter.Detach();
    }
    void CreatePipelineState(std::vector<D3D12_INPUT_ELEMENT_DESC> &inputDescs)
    {
        // Create an empty root signature.
        {
            D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};

            // This is the highest version the sample supports. If CheckFeatureSupport succeeds, the HighestVersion returned will not be greater than this.
            featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;

            if (FAILED(base->device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData))))
            {
                featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
            }

            CD3DX12_DESCRIPTOR_RANGE1 ranges[1];
            ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);

            CD3DX12_ROOT_PARAMETER1 rootParameters[2];
            rootParameters[0].InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_PIXEL);
            rootParameters[1].InitAsConstantBufferView(0);
            D3D12_STATIC_SAMPLER_DESC sampler = {};
            sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
            sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
            sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
            sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
            sampler.MipLODBias = 0;
            sampler.MaxAnisotropy = 0;
            sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
            sampler.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
            sampler.MinLOD = 0.0f;
            sampler.MaxLOD = D3D12_FLOAT32_MAX;
            sampler.ShaderRegister = 0;
            sampler.RegisterSpace = 0;
            sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

            CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
            rootSignatureDesc.Init_1_1(_countof(rootParameters), rootParameters, 1, &sampler, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

            ComPtr<ID3DBlob> signature;
            ComPtr<ID3DBlob> error;
            SK_CHECK_MSG(D3DX12SerializeVersionedRootSignature(&rootSignatureDesc,
                                                               featureData.HighestVersion,
                                                               &signature, &error),
                         error);
            SK_CHECK(base->device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&base->rootSignature)));
        }
        // Create the pipeline state, which includes compiling and loading shaders.
        {
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
                             GetAssetFullPath(L"shader/shader.hlsl").c_str(),
                             nullptr, nullptr, "VSMain", "vs_5_1",
                             compileFlags, 0, &vertexShader, &errorMessage),
                         errorMessage);

            SK_CHECK_MSG(D3DCompileFromFile(
                             GetAssetFullPath(L"shader/shader.hlsl").c_str(),
                             nullptr, nullptr, "PSMain", "ps_5_1",
                             compileFlags, 0, &pixelShader, &errorMessage),
                         errorMessage);

            // Describe and create the graphics pipeline state object (PSO).
            D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
            psoDesc.InputLayout = {inputDescs.data(), static_cast<uint32_t>(inputDescs.size())};
            psoDesc.pRootSignature = base->rootSignature.Get();
            psoDesc.VS = CD3DX12_SHADER_BYTECODE(vertexShader.Get());
            psoDesc.PS = CD3DX12_SHADER_BYTECODE(pixelShader.Get());
            auto rs =CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
            // rs.FillMode=D3D12_FILL_MODE_WIREFRAME;
            rs.CullMode=D3D12_CULL_MODE_NONE;
            psoDesc.RasterizerState = rs;
            psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
            psoDesc.DepthStencilState.DepthEnable = FALSE;
            psoDesc.DepthStencilState.StencilEnable = FALSE;
            psoDesc.SampleMask = UINT_MAX;
            psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
            psoDesc.NumRenderTargets = 1;
            psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
            psoDesc.SampleDesc.Count = 1;
            SK_CHECK(base->device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&base->pipelineState)));
        }
    }

public:
    void Init(SkBase *initBase)
    {
        base = initBase;
        InitSwapChain();
    }
    void Setup(std::vector<D3D12_INPUT_ELEMENT_DESC> &inputDescs)
    {
        CreatePipelineState(inputDescs);
    }
};
