#pragma once
#include "SkBase.h"
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
    SkBase *base;

public:
    ComPtr<ID3D12Resource> vertexBuf;
    D3D12_VERTEX_BUFFER_VIEW vertexBufView;
    ComPtr<ID3D12Resource> indexBuffer;
    D3D12_INDEX_BUFFER_VIEW indexBufView;
    uint32_t vertexCount = 0;
    uint32_t indexCount = 0;
    // Define the vertex input layout.
    std::vector<float> vertexData;
    std::vector<uint32_t> indexData;
    std::vector<SkSubMesh> subMeshes;
    void Init(SkBase *initBase)
    {
        base = initBase;
        subMeshes.clear();
        vertexData.clear();
        indexData.clear();
    }
    void SetupTriangle(std::vector<D3D12_INPUT_ELEMENT_DESC> &inputDescs)
    {

        struct Vertex
        {
            XMFLOAT3 position;
            XMFLOAT4 color;
            XMFLOAT2 uv;
        };
        inputDescs = {
            {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
            {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, offsetof(Vertex, color), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
            {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offsetof(Vertex, uv), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}};
        {
            // Define the geometry for a triangle.
            Vertex triangleVertices[] =
                {
                    {{0.0f, 0.5f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
                    {{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},
                    {{-0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, 1.0f, 1.0f}, {0.0f, 0.0f}}};
            this->vertexCount = 3;
            const UINT vertexBufferSize = sizeof(triangleVertices);

            // Note: using upload heaps to transfer static data like vert buffers is not
            // recommended. Every time the GPU needs it, the upload heap will be marshalled
            // over. Please read up on Default Heap usage. An upload heap is used here for
            // code simplicity and because there are very few verts to actually transfer.
            SK_CHECK(base->device->CreateCommittedResource(
                &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
                D3D12_HEAP_FLAG_NONE,
                &CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize),
                D3D12_RESOURCE_STATE_GENERIC_READ,
                nullptr,
                IID_PPV_ARGS(&vertexBuf)));

            // Copy the triangle data to the vertex buffer.
            void *pVertexDataBegin;
            CD3DX12_RANGE readRange(0, 0); // We do not intend to read from this resource on the CPU.
            SK_CHECK(vertexBuf->Map(0, &readRange, &pVertexDataBegin));
            memcpy(pVertexDataBegin, triangleVertices, sizeof(triangleVertices));
            vertexBuf->Unmap(0, nullptr);

            // Initialize the vertex buffer view.
            vertexBufView.BufferLocation = vertexBuf->GetGPUVirtualAddress();
            vertexBufView.StrideInBytes = sizeof(Vertex);
            vertexBufView.SizeInBytes = vertexBufferSize;
        }
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
            }
            else
            {
                cmd->DrawInstanced(this->vertexCount, 1, 0, 0);
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
