﻿#pragma once
#include "SkTools.h"
#include "SkCamera.h"
class SkBase
{
public:
    float timer;
    float delta;
    uint32_t iFrame;
    uint32_t width;
    uint32_t height;
    std::string name;
    HWND hwnd = nullptr;
    HINSTANCE hInstance;
    int iCmdShow;
    const uint32_t imageCount = 2;
    const DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM;
    const DXGI_FORMAT depthFormat = DXGI_FORMAT_D32_FLOAT;
    const float clearColor[4] = {0.0f, 0.0f, 0.0f, 0.0f};
    uint32_t imageIndex;
    ComPtr<IDXGISwapChain3> swapChain;
    ComPtr<ID3D12Device> device;
    std::vector<ComPtr<ID3D12Resource>> renderTargets;
    ComPtr<ID3D12CommandAllocator> cmdPool;
    ComPtr<ID3D12CommandQueue> cmdQueue;
    SkHeap *heap = nullptr;
    ComPtr<ID3D12Resource> depthTarget;

    SkCamera cam;
    // bool useWarpDevice = false;
    D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};
    const D3D12_STATIC_SAMPLER_DESC sampler=InitSampler();
    SkInclude include;

    HWND GetHwnd()
    {
        if (hwnd == nullptr)
        {
            throw std::runtime_error("Window not Init!");
        }
        return hwnd;
    }
};
