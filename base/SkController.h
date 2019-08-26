#pragma once
#include "SkBase.h"

class SkController
{
private:
    SkBase *base = nullptr;
    SkAgent *agent = nullptr;

public:
    SkBuffer uniBuf;
    struct UniformBuffer
    {
        Matrix projection;
        Matrix view;
    } uniformBuffer;
    D3D12_CONSTANT_BUFFER_VIEW_DESC bufDes;
    void Init(SkAgent *initAgent)
    {
        // base = initAgent;
        agent = initAgent;
        base = agent->GetBase();
        // uniformBuffer.projection = DirectX::XMMatrixPerspectiveFovLH(XMConvertToRadians(60.0f), (float)base->width / base->height, 0.1f, 100.0f);
        Vector3 eyePos{0.0f, 3.0f, -10.0f};
        Vector3 focusPos{0.0f, 0.0f, 0.0f};
        Vector3 up{0.0f, 1.0f, 0.0f};
        // TODO:
        Vector3 right=DirectX::XMVector3Cross(eyePos-focusPos,up);
        up=DirectX::XMVector3Cross(right,eyePos-focusPos);
        uniformBuffer.projection = Matrix::CreatePerspectiveFieldOfView(DirectX::XMConvertToRadians(90.0f), (float)base->width / base->height, 0.01f, 20.0f);
        uniformBuffer.projection = uniformBuffer.projection.Transpose();

        uniformBuffer.view = Matrix::CreateLookAt(eyePos, Vector3(0.0f), up);
        uniformBuffer.view=uniformBuffer.view.Transpose();
    }
    void Setup()
    {
        uint32_t bufSize = sizeof(UniformBuffer);
        SK_CHECK(agent->CreateBuffer(D3D12_HEAP_TYPE_UPLOAD,
                                     D3D12_HEAP_FLAG_NONE,
                                     bufSize, D3D12_RESOURCE_STATE_GENERIC_READ,
                                     nullptr, &uniBuf));
        SK_CHECK(uniBuf.Map());
        memcpy(uniBuf.data, &uniformBuffer, bufSize);
        uniBuf.Unmap();

        bufDes.BufferLocation = uniBuf.buf->GetGPUVirtualAddress();
        bufDes.SizeInBytes = uniBuf.bufSize;

    }
};
