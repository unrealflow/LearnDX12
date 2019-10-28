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
#include "d3dx12.h"
#include <chrono>
#include <string>
#include <vector>
#include <array>
#include <unordered_map>
#include <set>
#include <wrl.h>
#include <shellapi.h>
#include <SimpleMath.h>
#include <fstream>
#include <sstream>
// using namespace DirectX;
using namespace DirectX::SimpleMath;
using Microsoft::WRL::ComPtr;

struct SkBuffer
{
    ComPtr<ID3D12Resource> buf;
    uint32_t bufSize;
    void *data;
    HRESULT Map()
    {
        CD3DX12_RANGE range(0, bufSize);
        return buf->Map(0, &range, &data);
    }
    void Unmap()
    {
        buf->Unmap(0, nullptr);
    }
    void Load(void *src)
    {
        this->Map();
        memcpy(data,src,bufSize);
        this->Unmap();
    }
};
class SkCallback
{
public:
    virtual void WinProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) = 0;
};
class SkHeap;
class ISkPass
{
public:
    virtual void CmdSet(ComPtr<ID3D12GraphicsCommandList> cmd, uint32_t index)=0;
};