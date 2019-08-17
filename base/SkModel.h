#pragma once
#include "SkBase.h"
#include "SkMesh.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/cimport.h>
class SkModel
{

public:
    typedef enum Component
    {
        VERTEX_COMPONENT_POSITION = 0x0,
        VERTEX_COMPONENT_NORMAL = 0x1,
        VERTEX_COMPONENT_COLOR = 0x2,
        VERTEX_COMPONENT_UV = 0x3,
        VERTEX_COMPONENT_TANGENT = 0x4,
        VERTEX_COMPONENT_BITANGENT = 0x5,
        VERTEX_COMPONENT_DUMMY_FLOAT = 0x6,
        VERTEX_COMPONENT_DUMMY_VEC4 = 0x7,
        VERTEX_COMPONENT_MATINDEX = 0x8
    } Component;
    struct VertexLayout
    {
    public:
        /** @brief Components used to generate vertices from */
        std::vector<Component> components;
        VertexLayout()
        {
        }
        VertexLayout(std::vector<Component> components)
        {
            this->components = std::move(components);
        }

        uint32_t stride()
        {
            uint32_t res = 0;
            for (auto &component : components)
            {
                switch (component)
                {
                case VERTEX_COMPONENT_UV:
                    res += 2 * sizeof(float);
                    break;
                case VERTEX_COMPONENT_MATINDEX:
                    res += sizeof(float);
                    break;
                case VERTEX_COMPONENT_DUMMY_FLOAT:
                    res += sizeof(float);
                    break;
                case VERTEX_COMPONENT_DUMMY_VEC4:
                    res += 4 * sizeof(float);
                    break;
                default:
                    // All components except the ones listed above are made up of 3 floats
                    res += 3 * sizeof(float);
                }
            }
            return res;
        }
    };
    static XMFLOAT3 _XMFLOAT3(float x) { return XMFLOAT3{x, x, x}; }
    static XMFLOAT2 _XMFLOAT2(float x) { return XMFLOAT2{x, x}; }
    struct ModelCreateInfo
    {
        XMFLOAT3 center;
        XMFLOAT3 scale;
        XMFLOAT2 uvscale;
        // VkMemoryPropertyFlags memoryPropertyFlags = 0;

        ModelCreateInfo() : center(_XMFLOAT3(0.0f)), scale(_XMFLOAT3(1.0f)), uvscale(_XMFLOAT2(1.0f)){};

        ModelCreateInfo(XMFLOAT3 scale, XMFLOAT2 uvscale, XMFLOAT3 center)
        {
            this->center = center;
            this->scale = scale;
            this->uvscale = uvscale;
        }

        ModelCreateInfo(float scale, float uvscale, float center)
        {
            this->center = _XMFLOAT3(center);
            this->scale = _XMFLOAT3(scale);
            this->uvscale = _XMFLOAT2(uvscale);
        }
    };

private:
    SkBase *base;
    static const int defaultFlags = aiProcess_FlipWindingOrder | aiProcess_Triangulate | aiProcess_PreTransformVertices | aiProcess_CalcTangentSpace | aiProcess_GenSmoothNormals;
    SkMesh mesh;
    
};
