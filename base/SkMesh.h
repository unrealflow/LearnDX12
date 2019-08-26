#pragma once
#include "SkBase.h"
#include "SkAgent.h"
struct SkSubMesh
{
    uint32_t vertexBase;
    uint32_t vertexCount;
    uint32_t indexBase;
    uint32_t indexCount;
};
class SkMesh
{
private:
    // SkBase *base;
    SkAgent *agent;

public:
    // ComPtr<ID3D12Resource> vertexBuf;
    D3D12_VERTEX_BUFFER_VIEW vertexBufView;
    SkBuffer vertexBuf;
    SkBuffer indexBuf;
    D3D12_INDEX_BUFFER_VIEW indexBufView;
    uint32_t vertexCount = 0;
    uint32_t indexCount = 0;
    uint32_t stride = 0;
    // Define the vertex input layout.
    std::vector<float> vertexData;
    std::vector<uint32_t> indexData;
    std::vector<SkSubMesh> subMeshes;
    void Init(SkAgent *initAgent)
    {
        // base = initBase;
        agent=initAgent;
        subMeshes.clear();
        vertexData.clear();
        indexData.clear();
    }
    void SetupTriangle(std::vector<D3D12_INPUT_ELEMENT_DESC> &inputDescs)
    {

        struct Vertex
        {
            Vector3 position;
            Vector3 normal;
            Vector2 uv;
        };
        inputDescs = {
            {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
            {"NORMAL", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, offsetof(Vertex, normal), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
            {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offsetof(Vertex, uv), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}};
        {
            // Define the geometry for a triangle.
            Vertex triangleVertices[] =
                {
                    {{0.0f, 0.5f, -0.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}},
                    {{0.5f, -0.5f, -0.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
                    {{-0.5f, -0.5f, -0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}}};
            this->vertexCount = 3;
            const UINT vertexBufferSize = sizeof(triangleVertices);

            SK_CHECK(agent->CreateBuffer(D3D12_HEAP_TYPE_UPLOAD,
                                        D3D12_HEAP_FLAG_NONE,
                                        vertexBufferSize,
                                        D3D12_RESOURCE_STATE_GENERIC_READ,
                                        nullptr,
                                        &vertexBuf));
            // Copy the triangle data to the vertex buffer.
            // CD3DX12_RANGE readRange(0, 0); // We do not intend to read from this resource on the CPU.
            SK_CHECK(vertexBuf.Map());
            memcpy(vertexBuf.data, triangleVertices, sizeof(triangleVertices));
            vertexBuf.Unmap();

            // Initialize the vertex buffer view.
            vertexBufView.BufferLocation = vertexBuf.buf->GetGPUVirtualAddress();
            vertexBufView.StrideInBytes = sizeof(Vertex);
            vertexBufView.SizeInBytes = vertexBuf.bufSize;
        }
    }
    void Setup()
    {
        uint32_t vertexBufferSize = (uint32_t)vertexData.size() * sizeof(float);
        SK_CHECK(agent->CreateBuffer(D3D12_HEAP_TYPE_UPLOAD,
                                    D3D12_HEAP_FLAG_NONE,
                                    vertexBufferSize,
                                    D3D12_RESOURCE_STATE_GENERIC_READ,
                                    nullptr,
                                    &vertexBuf));
        SK_CHECK(vertexBuf.Map());
        memcpy(vertexBuf.data, vertexData.data(), vertexBufferSize);
        vertexBuf.Unmap();

        vertexBufView.BufferLocation = vertexBuf.buf->GetGPUVirtualAddress();
        vertexBufView.StrideInBytes = this->stride;
        vertexBufView.SizeInBytes = vertexBuf.bufSize;

        uint32_t indexBufferSize = (uint32_t)indexData.size() * sizeof(uint32_t);
        SK_CHECK(agent->CreateBuffer(D3D12_HEAP_TYPE_UPLOAD,
                                    D3D12_HEAP_FLAG_NONE,
                                    indexBufferSize,
                                    D3D12_RESOURCE_STATE_GENERIC_READ,
                                    nullptr,
                                    &indexBuf));
        SK_CHECK(indexBuf.Map());
        memcpy(indexBuf.data, indexData.data(), indexBufferSize);
        indexBuf.Unmap();

        indexBufView.BufferLocation = indexBuf.buf->GetGPUVirtualAddress();
        indexBufView.Format = DXGI_FORMAT_R32_UINT;
        indexBufView.SizeInBytes = indexBuf.bufSize;
    }
    void Draw(ID3D12GraphicsCommandList *cmd)
    {
        cmd->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        if (vertexCount > 0)
        {
            cmd->IASetVertexBuffers(0, 1, &vertexBufView);
            if (indexCount > 0)
            {
                cmd->IASetIndexBuffer(&indexBufView);
                cmd->DrawIndexedInstanced(this->indexCount, 1, 0, 0, 0);
                fprintf(stderr, "Draw by Indices...\n");
            }
            else
            {
                cmd->DrawInstanced(this->vertexCount, 1, 0, 0);
                fprintf(stderr, "Draw by Vertices...\n");
            }
        }
        else
        {
            cmd->DrawInstanced(3, 1, 0, 0);
        }
    }
    void DrawSubMesh(ID3D12GraphicsCommandList *cmd, uint32_t index)
    {
        cmd->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        cmd->IASetVertexBuffers(0, 1, &vertexBufView);
        if (indexCount > 0)
        {
            cmd->IASetIndexBuffer(&indexBufView);
            cmd->DrawIndexedInstanced(subMeshes[index].indexCount,
                                      1, subMeshes[index].indexBase, 0, 0);
        }
        else
        {
            cmd->DrawInstanced(subMeshes[index].vertexCount, 1, subMeshes[index].vertexBase, 0);
        }
    }
};
