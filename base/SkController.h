#pragma once
#include "SkBase.h"
#include "SkAgent.h"
class SkController : public SkCallback
{
private:
    SkBase *base = nullptr;
    SkAgent *agent = nullptr;
    SkCamera *cam = nullptr;
    const float _WalkSpeed = 0.01f;

public:
    SkBuffer uniBuf;
    struct UniformBuffer
    {
        Matrix projection;
        Matrix view;
    } uniformBuffer;
    struct
    {
        bool left = false;
        bool mid = false;
        bool right = false;
        Vector2 pos;
        LPARAM lParam;
    } mouse;

    D3D12_CONSTANT_BUFFER_VIEW_DESC bufDes;
    void Init(SkAgent *initAgent)
    {
        // base = initAgent;
        agent = initAgent;
        base = agent->GetBase();
        cam = &base->cam;
        Vector3 eyePos{0.0f, 3.0f, -10.0f};
        Vector3 focusPos{0.0f, 0.0f, 0.0f};
        Vector3 up{0.0f, 1.0f, 0.0f};
        // TODO:
        Vector3 right = DirectX::XMVector3Cross(eyePos - focusPos, up);
        up = DirectX::XMVector3Cross(right, eyePos - focusPos);
        cam->SetLens(DirectX::XMConvertToRadians(60.0f), (float)base->width / base->height, 0.01f, 20.0f);
        cam->SetLookAt(eyePos, Vector3(0.0f), up);
        // uniformBuffer.projection = Matrix::CreatePerspectiveFieldOfView(DirectX::XMConvertToRadians(90.0f), (float)base->width / base->height, 0.01f, 20.0f);
        // uniformBuffer.view = Matrix::CreateLookAt(eyePos, Vector3(0.0f), up);
        uniformBuffer.projection = cam->proj.Transpose();
        uniformBuffer.view = cam->view.Transpose();
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
    void Update()
    {
        cam->Walking(base->delta);
        MouseProc();
        uniformBuffer.projection = cam->proj.Transpose();
        uniformBuffer.view = cam->view.Transpose();
        SK_CHECK(uniBuf.Map());
        memcpy(uniBuf.data, &uniformBuffer, uniBuf.bufSize);
        uniBuf.Unmap();
    }
    void MouseProc()
    {
        float x=(float)LOWORD(mouse.lParam)/base->width;
        float y=(float)HIWORD(mouse.lParam)/base->height;
        float dx = mouse.pos.x - x;
        float dy = mouse.pos.y - y;
        if (mouse.left)
        {
            cam->Turn(dx, dy);
        }
        mouse.pos.x = x;
        mouse.pos.y = y;
    }
    void WinProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override
    {
        switch (uMsg)
        {
        case WM_KEYDOWN:
            Event_Key(wParam, true);
            break;
        case WM_KEYUP:
            Event_Key(wParam, false);
            break;
        case WM_LBUTTONDOWN:
            Event_LButton(wParam, true);
            break;
        case WM_LBUTTONUP:
            Event_LButton(wParam, false);
            break;
        case WM_MOUSEMOVE:
            mouse.lParam = lParam;
            break;
        case WM_MOUSELEAVE:
            break;
        default:
            break;
        }
    }
    void Event_Key(WPARAM wParam, bool isDown)
    {
        switch (wParam)
        {
        case 'W':
        case 'w':
            cam->keys.front = isDown;
            break;
        case 'S':
        case 's':
            cam->keys.back = isDown;
            break;
        case 'A':
        case 'a':
            cam->keys.left = isDown;
            break;
        case 'D':
        case 'd':
            cam->keys.right = isDown;
            break;
        default:
            break;
        }
    }
    void Event_LButton(WPARAM wParam, bool isDown)
    {
        mouse.left = isDown;
    }
};
