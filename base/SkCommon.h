#pragma once
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN // Exclude rarely-used stuff from Windows headers.
#endif
#include <windows.h>
#include <stdexcept>
#include <iostream>

#include <d3d12.h>
#include <dxgi1_4.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>
#include "d3dx12.h"

#include <string>
#include <wrl.h>
#include <shellapi.h>
using namespace DirectX;
using Microsoft::WRL::ComPtr;

struct SkBuffer
{
    ComPtr<ID3D12Resource> buf;
    uint32_t bufSize;
    void *data;
    HRESULT Map()
    {
        CD3DX12_RANGE range(0,bufSize);
        return buf->Map(0,&range,&data);
    }
    void Unmap()
    {
        buf->Unmap(0,nullptr);
    }
};