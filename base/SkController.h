#pragma once
#include "SkBase.h"

class SkController
{
private:
    SkBase *base = nullptr;

public:
    SkBuffer uniBuf;
    struct UniformBuffer
    {
        XMMATRIX projection;
        XMMATRIX view;
    } uniformBuffer;
    D3D12_CONSTANT_BUFFER_VIEW_DESC bufDes;
    void Init(SkBase *initBase)
    {
        base = initBase;
        // uniformBuffer.projection = DirectX::XMMatrixPerspectiveFovLH(XMConvertToRadians(60.0f), (float)base->width / base->height, 0.1f, 100.0f);
        XMVECTOR eyePos{0.0f, 10.0f, -5.0f};
        XMVECTOR focusPos{0.0f, 0.0f, 0.0f};
        XMVECTOR up{0.0f, 1.0f, 0.0f};
        XMVECTOR tmp=DirectX::XMVector3Cross(eyePos-focusPos,up);
        up=DirectX::XMVector3Cross(tmp,eyePos-focusPos);
        uniformBuffer.projection = DirectX::XMMatrixLookAtLH(eyePos,
                                                             focusPos,
                                                             up);
        uniformBuffer.view = XMMATRIX();
    }
    void Setup()
    {
        uint32_t bufSize = sizeof(UniformBuffer);
        SK_CHECK(base->CreateBuffer(D3D12_HEAP_TYPE_UPLOAD,
                                    D3D12_HEAP_FLAG_NONE,
                                    bufSize, D3D12_RESOURCE_STATE_GENERIC_READ,
                                    nullptr, &uniBuf));
        SK_CHECK(uniBuf.Map());
        memcpy(uniBuf.data, &uniformBuffer, bufSize);
        uniBuf.Unmap();

        bufDes.BufferLocation = uniBuf.buf->GetGPUVirtualAddress();
        bufDes.SizeInBytes = uniBuf.bufSize;
        // {
        //     auto cmd=base->BeginCmd();
        //     cmd->SetGraphicsRootConstantBufferView(0,uniBuf.buf->GetGPUVirtualAddress());
        //     base->FlushCmd(cmd);
        // }
    }
};
