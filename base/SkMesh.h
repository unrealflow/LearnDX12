#pragma once
#include "SkBase.h"

class SkMesh
{
private:
    SkBase *base;

public:
    ComPtr<ID3D12Resource> m_vertexBuffer;
    D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;
    std::vector<D3D12_INPUT_ELEMENT_DESC> inputDescs;
    // Define the vertex input layout.

    void Init(SkBase *initBase)
    {
        base = initBase;
    }
    void SetupTriangle()
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
                    {{0.0f, 0.5f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},
                    {{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
                    {{-0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, 1.0f, 1.0f}, {0.0f, 1.0f}}};

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
                IID_PPV_ARGS(&m_vertexBuffer)));

            // Copy the triangle data to the vertex buffer.
            void *pVertexDataBegin;
            CD3DX12_RANGE readRange(0, 0); // We do not intend to read from this resource on the CPU.
            SK_CHECK(m_vertexBuffer->Map(0, &readRange, &pVertexDataBegin));
            memcpy(pVertexDataBegin, triangleVertices, sizeof(triangleVertices));
            m_vertexBuffer->Unmap(0, nullptr);

            // Initialize the vertex buffer view.
            m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
            m_vertexBufferView.StrideInBytes = sizeof(Vertex);
            m_vertexBufferView.SizeInBytes = vertexBufferSize;
        }
    }
    void Draw(ID3D12GraphicsCommandList *cmd)
    {
        cmd->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        cmd->IASetVertexBuffers(0, 1, &m_vertexBufferView);
        cmd->DrawInstanced(3, 1, 0, 0);
    }
};
