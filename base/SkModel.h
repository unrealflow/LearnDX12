#pragma once
#include "SkBase.h"
#include "SkMesh.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/cimport.h>

// #define vec3(x) \
//     aiVector3D { x, x, x }
// #define vec2(x) \
//     aiVector2D { x, x }
#define vec3 Vector3
#define vec2 Vector2
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
        VERTEX_COMPONENT_DUMMYVEC4 = 0x7,
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
                case VERTEX_COMPONENT_DUMMYVEC4:
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

    struct ModelCreateInfo
    {
        vec3 center;
        vec3 scale;
        vec2 uvscale;
        // VkMemoryPropertyFlags memoryPropertyFlags = 0;

        ModelCreateInfo() : center(vec3(0.0f)), scale(vec3(1.0f)), uvscale(vec2(1.0f)){};

        ModelCreateInfo(vec3 scale, vec2 uvscale, vec3 center)
        {
            this->center = center;
            this->scale = scale;
            this->uvscale = uvscale;
        }

        ModelCreateInfo(float scale, float uvscale, float center)
        {
            this->center = vec3(center);
            this->scale = vec3(scale);
            this->uvscale = vec2(uvscale);
        }
    };

private:
    // SkBase *base;
    SkAgent *agent;
    static const int defaultFlags = aiProcess_ConvertToLeftHanded| aiProcess_FlipWindingOrder | aiProcess_Triangulate | aiProcess_PreTransformVertices | aiProcess_CalcTangentSpace | aiProcess_GenSmoothNormals;

public:
    SkMesh mesh;
    VertexLayout layout;
    // uint32_t indexCount = 0;
    // uint32_t vertexCount = 0;
    std::vector<D3D12_INPUT_ELEMENT_DESC> inputDescs;
    struct Dimension
    {
        vec3 min = vec3(FLT_MAX);
        vec3 max = vec3(-FLT_MAX);
        vec3 size;
    } dim;

    void Init(SkAgent*initAgent)
    {
        // base = initBase;
        agent=initAgent;
        mesh.Init(agent);
        // matSet.Init(mem);
        layout = {{
            VERTEX_COMPONENT_POSITION,
            VERTEX_COMPONENT_NORMAL,
            VERTEX_COMPONENT_UV,
        }};
        RebuildInputDescription();
    }
    //根据设置生成InputBindingDescription
    void RebuildInputDescription(UINT semanticIndex = 0, UINT inputSlot = 0)
    {
        inputDescs.resize(layout.components.size());
        uint32_t _offset = 0;
        for (size_t i = 0; i < layout.components.size(); i++)
        {
            switch (layout.components[i])
            {
            case VERTEX_COMPONENT_NORMAL:
                inputDescs[i] = {"NORMAL",
                                 semanticIndex,
                                 DXGI_FORMAT_R32G32B32_FLOAT,
                                 inputSlot,
                                 _offset,
                                 D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
                                 0};
                _offset += sizeof(float) * 3;
                break;
            case VERTEX_COMPONENT_COLOR:
                inputDescs[i] = {"COLOR",
                                 semanticIndex,
                                 DXGI_FORMAT_R32G32B32_FLOAT,
                                 inputSlot,
                                 _offset,
                                 D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
                                 0};
                _offset += sizeof(float) * 3;
                break;
            case VERTEX_COMPONENT_POSITION:
                // inputAttributes[i].format = VK_FORMAT_R32G32B32_SFLOAT;
                inputDescs[i] = {"POSITION",
                                 semanticIndex,
                                 DXGI_FORMAT_R32G32B32_FLOAT,
                                 inputSlot,
                                 _offset,
                                 D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
                                 0};
                _offset += sizeof(float) * 3;
                break;
            case VERTEX_COMPONENT_UV:
                inputDescs[i] = {"TEXCOORD",
                                 semanticIndex,
                                 DXGI_FORMAT_R32G32_FLOAT,
                                 inputSlot,
                                 _offset,
                                 D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
                                 0};
                _offset += sizeof(float) * 2;
                break;
            default:
                throw std::runtime_error("Components Error!");
                break;
            }
        }
    }
    void ImportModel(const std::string &path, ModelCreateInfo *createInfo = nullptr, VertexLayout *_layout = nullptr)
    {
        Assimp::Importer importer;
        const aiScene *pScene;
        pScene = importer.ReadFile(path, defaultFlags);
        if (!pScene)
        {
            std::string error = importer.GetErrorString();
            throw std::runtime_error("Can't load " + path + "! " + error);
        }

        std::string directory;
        switch (path[0])
        {
        case 'C':
        case 'D':
        case 'E':
        case 'F':
        case 'G':
        case 'Z':
        case '.':
            if (path[1] == '/' || path[1] == '\\')
            {
                directory = path.substr(0, path.find_last_of('/'));
            }
            break;
        default:
            directory = ".";
            break;
        }
        if (pScene)
        {
            vec3 scale = vec3(1.0f);
            vec2 uvscale = vec2(1.0f);
            vec3 center = vec3(0.0f);
            if (createInfo)
            {
                scale = createInfo->scale;
                uvscale = createInfo->uvscale;
                center = createInfo->center;
            }
            mesh.vertexCount = 0;
            mesh.indexCount = 0;
            mesh.stride=layout.stride();
            if (_layout != nullptr)
            {
                this->layout = VertexLayout(*_layout);
                RebuildInputDescription();
            }
            mesh.subMeshes.clear();
            mesh.subMeshes.resize(pScene->mNumMeshes);
            for (uint32_t i = 0; i < pScene->mNumMeshes; i++)
            {
                const aiMesh *paiMesh = pScene->mMeshes[i];

                mesh.subMeshes[i] = {};
                mesh.subMeshes[i].vertexBase = mesh.vertexCount;
                mesh.subMeshes[i].indexBase = mesh.indexCount;

                mesh.vertexCount += paiMesh->mNumVertices;

                aiColor3D pColor(1.0f, 1.0f, 1.0f);
                aiMaterial *material = pScene->mMaterials[paiMesh->mMaterialIndex];
                material->Get(AI_MATKEY_COLOR_DIFFUSE, pColor);
                const aiVector3D Zero3D(0.0f, 0.0f, 0.0f);
                for (unsigned int j = 0; j < paiMesh->mNumVertices; j++)
                {
                    const aiVector3D *pPos = &(paiMesh->mVertices[j]);
                    const aiVector3D *pNormal = &(paiMesh->mNormals[j]);
                    const aiVector3D *pTexCoord = (paiMesh->HasTextureCoords(0)) ? &(paiMesh->mTextureCoords[0][j]) : &Zero3D;
                    const aiVector3D *pTangent = (paiMesh->HasTangentsAndBitangents()) ? &(paiMesh->mTangents[j]) : &Zero3D;
                    const aiVector3D *pBiTangent = (paiMesh->HasTangentsAndBitangents()) ? &(paiMesh->mBitangents[j]) : &Zero3D;

                    for (auto &component : layout.components)
                    {
                        switch (component)
                        {
                        case VERTEX_COMPONENT_POSITION:
                            mesh.vertexData.push_back(pPos->x * scale.x + center.x);
                            mesh.vertexData.push_back(pPos->y * scale.y + center.y);
                            mesh.vertexData.push_back(-pPos->z * scale.z + center.z);
                            break;
                        case VERTEX_COMPONENT_NORMAL:
                            mesh.vertexData.push_back(pNormal->x);
                            mesh.vertexData.push_back(pNormal->y);
                            mesh.vertexData.push_back(-pNormal->z);
                            break;
                        case VERTEX_COMPONENT_UV:
                            mesh.vertexData.push_back(pTexCoord->x * uvscale.x);
                            mesh.vertexData.push_back(pTexCoord->y * uvscale.y);
                            break;
                        case VERTEX_COMPONENT_COLOR:
                            mesh.vertexData.push_back(pColor.r);
                            mesh.vertexData.push_back(pColor.g);
                            mesh.vertexData.push_back(pColor.b);
                            break;
                        case VERTEX_COMPONENT_TANGENT:
                            mesh.vertexData.push_back(pTangent->x);
                            mesh.vertexData.push_back(pTangent->y);
                            mesh.vertexData.push_back(pTangent->z);
                            break;
                        case VERTEX_COMPONENT_BITANGENT:
                            mesh.vertexData.push_back(pBiTangent->x);
                            mesh.vertexData.push_back(pBiTangent->y);
                            mesh.vertexData.push_back(pBiTangent->z);
                            break;
                        // Dummy components for padding
                        case VERTEX_COMPONENT_DUMMY_FLOAT:
                            mesh.vertexData.push_back(0.0f);
                            break;
                        case VERTEX_COMPONENT_DUMMYVEC4:
                            mesh.vertexData.push_back(0.0f);
                            mesh.vertexData.push_back(0.0f);
                            mesh.vertexData.push_back(0.0f);
                            mesh.vertexData.push_back(0.0f);
                            break;
                        };
                    }
                    dim.max.x = fmax(pPos->x, dim.max.x);
                    dim.max.y = fmax(pPos->y, dim.max.y);
                    dim.max.z = fmax(pPos->z, dim.max.z);

                    dim.min.x = fmin(pPos->x, dim.min.x);
                    dim.min.y = fmin(pPos->y, dim.min.y);
                    dim.min.z = fmin(pPos->z, dim.min.z);
                }
                dim.size = dim.max - dim.min;
                mesh.subMeshes[i].vertexCount = paiMesh->mNumVertices;

                uint32_t indexBase = static_cast<uint32_t>(mesh.indexData.size());
                for (unsigned int j = 0; j < paiMesh->mNumFaces; j++)
                {
                    const aiFace &Face = paiMesh->mFaces[j];
                    if (Face.mNumIndices != 3)
                        continue;
                    mesh.indexData.push_back(mesh.subMeshes[i].vertexBase + Face.mIndices[0]);
                    mesh.indexData.push_back(mesh.subMeshes[i].vertexBase + Face.mIndices[1]);
                    mesh.indexData.push_back(mesh.subMeshes[i].vertexBase + Face.mIndices[2]);
                    mesh.subMeshes[i].indexCount += 3;
                    mesh.indexCount += 3;
                }
            }
        }
    }
};
