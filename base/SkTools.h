#pragma once
#include "SkCommon.h"

#define SK_CHECK(_res_)                                                                \
    {                                                                                  \
        HRESULT _h_res_ = _res_;                                                       \
        if (FAILED(_h_res_))                                                           \
        {                                                                              \
            fprintf(stderr, "ERROR: HRESULT of 0x%08X\t", static_cast<UINT>(_h_res_)); \
            std::cout << "In " << __FILE__ << " at line " << __LINE__ << std::endl;    \
            throw std::runtime_error("HRESULT");                                       \
        }                                                                              \
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
            throw std::runtime_error("HRESULT");                                       \
        }                                                                              \
    }
#ifdef _DEBUG
#define SK_DEBUG()                                                    \
    {                                                                 \
        fprintf(stderr, "In %s at line %d...\n", __FILE__, __LINE__); \
    }
#else
#define SK_DEBUG() \
    {              \
    }
#endif

#define SK_N_NULL(_res_)                              \
    {                                                 \
        bool _h_res_ = (_res_);                       \
        if (!_h_res_)                                 \
        {                                             \
            SK_DEBUG();                               \
            throw std::runtime_error("Null Pointer"); \
        }                                             \
    }

#ifdef SK_DATA_DIR_W
const std::wstring wAssetsPath = SK_DATA_DIR_W;
#else
const std::wstring wAssetsPath = L"./";
#endif

#ifdef SK_DATA_DIR
const std::string AssetsPath = SK_DATA_DIR;
#else
const std::string AssetsPath = "./";
#endif

inline std::wstring GetAssetFullPath(LPCWSTR assetName)
{
    auto path = wAssetsPath + assetName;
    std::wcout << path << std::endl;
    return path;
}
inline std::string GetAssetFullPath(std::string assetName)
{
    auto path = AssetsPath + assetName;
    std::cout << path << std::endl;
    return path;
}
static auto StartTime= std::chrono::steady_clock::now();
inline float GetMilliTime()
{
    auto fp=std::chrono::steady_clock::now();
    return std::chrono::duration_cast<std::chrono::microseconds>(fp-StartTime).count()/1000.0f;
}