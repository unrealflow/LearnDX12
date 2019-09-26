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

inline std::wstring GetAssetFullPath(std::wstring assetName)
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
static auto StartTime = std::chrono::steady_clock::now();
inline float GetMilliTime()
{
    auto fp = std::chrono::steady_clock::now();
    return std::chrono::duration_cast<std::chrono::microseconds>(fp - StartTime).count() / 1000.0f;
}
inline void Show(Matrix &m)
{
    fprintf(stderr, "\t[\n");
    for (size_t i = 0; i < 4; i++)
    {
        fprintf(stderr, "\t[%f\t%f\t%f\t%f]\n", m.m[i][0], m.m[i][1], m.m[i][2], m.m[i][3]);
    }
    fprintf(stderr, "\t]\n");
}
inline void Show(Vector4 &v)
{
    fprintf(stderr, "\t[%f\t%f\t%f\t%f]\n", v.x, v.y, v.z, v.w);
}
inline void Show(Vector3 &v)
{
    fprintf(stderr, "\t[%f\t%f\t%f]\n", v.x, v.y, v.z);
}
inline Vector4 Mul(Vector4 &v, Matrix &m)
{
    Vector4 r;
    r.x = v.x * m._11 + v.y * m._21 + v.z * m._31 + v.w * m._41;
    r.y = v.x * m._12 + v.y * m._22 + v.z * m._32 + v.w * m._42;
    r.z = v.x * m._13 + v.y * m._23 + v.z * m._33 + v.w * m._43;
    r.w = v.x * m._14 + v.y * m._24 + v.z * m._34 + v.w * m._44;
    return r;
}
inline Vector4 Mul(Matrix &m, Vector4 &v)
{
    Vector4 r;
    r.x = v.x * m._11 + v.y * m._12 + v.z * m._13 + v.w * m._14;
    r.y = v.x * m._21 + v.y * m._22 + v.z * m._23 + v.w * m._24;
    r.z = v.x * m._31 + v.y * m._32 + v.z * m._33 + v.w * m._34;
    r.w = v.x * m._41 + v.y * m._42 + v.z * m._43 + v.w * m._44;
    return r;
}
inline Vector4 DivW(Vector4 &v)
{
    v = v / v.w;
    return v;
}
inline D3D12_STATIC_SAMPLER_DESC InitSampler()
{
    D3D12_STATIC_SAMPLER_DESC sampler;
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
    return sampler;
}
class SkInclude : public ID3DInclude
{
private:
    std::string shader_dir = "shader/";
    std::unordered_map<std::string, std::pair<char *, uint32_t>> name_to_data;

public:
    SkInclude()
    {
        name_to_data.clear();
    }
    SkInclude(std::string dir)
    {
        shader_dir = dir;
        name_to_data.clear();
    }
    HRESULT __stdcall Open(D3D_INCLUDE_TYPE IncludeType, LPCSTR pFileName, LPCVOID pParentData, LPCVOID *ppData, UINT *pBytes)
    {
        if (name_to_data.find(pFileName) != name_to_data.end())
        { 
            auto p = name_to_data[pFileName];
            *ppData = p.first;
            *pBytes = p.second;
            return S_OK;
        }
        std::string code = "";
        std::ifstream file;
        // ensure ifstream objects can throw exceptions:
        file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        try
        {
            // open files
            file.open(GetAssetFullPath(shader_dir + pFileName));
            std::stringstream vShaderStream;
            vShaderStream << file.rdbuf();
            file.close();
            code = vShaderStream.str();
        }
        catch (std::ifstream::failure e)
        {
            std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
        }
        if (code.length() > 0)
        {
            char *pData = new char[code.length() + 10];
            strcpy_s(pData, code.length() + 1, code.c_str());
            uint32_t len = static_cast<UINT>(code.length());
            name_to_data[pFileName] = std::pair(pData, len);
            *ppData = pData;
            *pBytes = len;
        }
        return S_OK;
    }
    HRESULT __stdcall Close(LPCVOID pData)
    {
        return S_OK;
    }
    ~SkInclude()
    {
        for (auto &&p : name_to_data)
        {           
            delete [] p.second.first;
            p.second.first=nullptr;
        }
    }
};