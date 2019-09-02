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
        swapChainDesc.Format = base->format;
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
            rtvHeapDesc.NumDescriptors = base->imageCount + 5;
            rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
            rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
            SK_CHECK(base->device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&base->rtvHeap)));

            D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
            srvHeapDesc.NumDescriptors = 5;
            srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
            srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
            SK_CHECK(base->device->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&base->srvHeap)));

            D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
            srvHeapDesc.NumDescriptors = 2;
            srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
            srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
            SK_CHECK(base->device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&base->dsvHeap)));

            base->rtvDesSize = base->device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
            base->srvDesSize = base->device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
            base->dsvDesSize = base->device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
            fprintf(stderr, "base->rtvDesSize:%d...\n", base->rtvDesSize);
            fprintf(stderr, "base->srvDesSize:%d...\n", base->srvDesSize);
            fprintf(stderr, "base->dsvDesSize:%d...\n", base->dsvDesSize);
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

            D3D12_RESOURCE_DESC depthDes = {};
            depthDes.MipLevels = 1;
            depthDes.Format = DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
            depthDes.Width = base->width;
            depthDes.Height = base->height;
            depthDes.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
            depthDes.DepthOrArraySize = 1;
            depthDes.SampleDesc.Count = 1;
            depthDes.SampleDesc.Quality = 0;
            depthDes.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;

            D3D12_CLEAR_VALUE clearValue = {};
            clearValue.Format = depthDes.Format;
            clearValue.DepthStencil = {1.0f, 0};
            SK_CHECK(base->device->CreateCommittedResource(
                &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
                D3D12_HEAP_FLAG_NONE, &depthDes,
                D3D12_RESOURCE_STATE_COMMON, &clearValue, IID_PPV_ARGS(&base->depthTarget)));

            D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
            dsvDesc.Format = depthDes.Format;
            dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
            dsvDesc.Texture2D = {0};
            // TODO :
            // base->device->CreateDepthStencilView(base->depthTarget.Get(), &dsvDesc, base->dsvHeap->GetCPUDescriptorHandleForHeapStart());
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

public:
    void Init(SkBase *initBase)
    {
        base = initBase;
        InitSwapChain();
    }
};
