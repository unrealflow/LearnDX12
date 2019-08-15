#pragma once
#include "SkCommon.h"

#define SK_CHECK(_res_)                                                               \
    {                                                                                 \
        HRESULT h_res_ = _res_;                                                       \
        if (FAILED(h_res_))                                                           \
        {                                                                             \
            fprintf(stderr, "ERROR: HRESULT of 0x%08X\t", static_cast<UINT>(h_res_)); \
            std::cout << "In " << __FILE__ << " at line " << __LINE__ << std::endl;   \
            throw std::runtime_error("ERROR");                                        \
        }                                                                             \
    }
#define SK_CHECK_MSG(_res_, _msg_)                                                     \
    {                                                                                  \
        HRESULT _h_res_ = _res_;                                                       \
        if (FAILED(_h_res_))                                                           \
        {                                                                              \
            if (_msg_.Get() != nullptr)                                                \
            {                                                                          \
                fprintf(stderr, "ERROR: %s", (char *)_msg_->GetBufferPointer());       \
            }                                                                          \
            fprintf(stderr, "ERROR: HRESULT of 0x%08X\t", static_cast<UINT>(_h_res_)); \
            std::cout << "In " << __FILE__ << " at line " << __LINE__ << std::endl;    \
            throw std::runtime_error("ERROR");                                         \
        }                                                                              \
    }

#ifdef SK_DATA_DIR
const std::wstring AssetsPath = SK_DATA_DIR;
#else
const std::wstring AssetsPath = L"./";
#endif

inline std::wstring GetAssetFullPath(LPCWSTR assetName)
{
    auto path = AssetsPath + assetName;
    // printf("%ws\n", path.c_str());
    std::wcout << path << std::endl;
    return path;
}