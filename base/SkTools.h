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

#define SK_CHECK(_res_)                                                            \
    {                                                                              \
        HRESULT h_res_ = _res_;                                                    \
        if (FAILED(h_res_))                                                        \
        {                                                                          \
            fprintf(stderr, "HRESULT of 0x%08X\t", static_cast<UINT>(h_res_));    \
            std::cout << "In " << __FILE__ << " at line " << __LINE__ << std::endl; \
            assert(false);                                                         \
        }                                                                          \
    }
