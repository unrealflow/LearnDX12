#pragma once
#include "SkAgent.h"
#include "assimp/scene.h"
#define MAX_MESH_COUNT 4
struct SkMat
{
    Vector3 baseColor;
    float roughness;
    float metallic;

    Vector3 _PAD_;
    SkMat()
    {
        baseColor = Vector3(1.0f);
        roughness = 0.3f;
        metallic = 0.3f;
    }
};
class SkMatSet
{
private:
    SkBase *base;
    SkAgent *agent;
    std::vector<SkMat> matSet;

    struct MatInfo
    {
        uint32_t PrimCounts[MAX_MESH_COUNT];
        uint32_t UseTex[MAX_MESH_COUNT];
    } matInfo;

public:
    SkBuffer matBuf;
    SkBuffer infoBuf;
    void Init(SkAgent *initAgent)
    {
        base = initAgent->GetBase();
        agent = initAgent;
        memset(&matInfo, 0, sizeof(MatInfo));
    }
    int AddMat(aiMaterial *aiMat, int PrimCount)
    {
        SkMat mat;
        aiColor3D pColor;
        aiMat->Get(AI_MATKEY_COLOR_DIFFUSE, pColor);
        mat.baseColor.x = pColor.r;
        mat.baseColor.y = pColor.g;
        mat.baseColor.z = pColor.b;
        matSet.emplace_back(mat);
        int index = static_cast<int>(matSet.size()) - 1;
        int texCount=aiMat->GetTextureCount(aiTextureType_DIFFUSE);
        if(texCount>0)
        {
            //TODO: Implement dynamic loading of textures
            matInfo.UseTex[index]=1;
        }
        matInfo.PrimCounts[index] = PrimCount;
        return index;
    }
    void Setup()
    {
        uint32_t matBufSize = sizeof(SkMat) * static_cast<uint32_t>(matSet.size());
        agent->CreateBuffer(D3D12_HEAP_TYPE_UPLOAD,
                            D3D12_HEAP_FLAG_NONE,
                            matBufSize,
                            D3D12_RESOURCE_STATE_GENERIC_READ,
                            nullptr,
                            &matBuf);
        matBuf.Load(matSet.data());

        agent->CreateBuffer(D3D12_HEAP_TYPE_UPLOAD,
                            D3D12_HEAP_FLAG_NONE,
                            sizeof(MatInfo),
                            D3D12_RESOURCE_STATE_GENERIC_READ,
                            nullptr,
                            &infoBuf);
        infoBuf.Load(&matInfo);
    }
};