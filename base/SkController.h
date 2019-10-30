#pragma once
#include "SkBase.h"
#include "SkAgent.h"
class SkController : public SkCallback
{
private:
    SkBase *base = nullptr;
    SkAgent *agent = nullptr;
    SkCamera *cam = nullptr;

public:
    SkBuffer uniBuf;
    struct UniformBuffer
    {
        Matrix projection;
        Matrix view;
        Matrix jitterProj;
        Matrix preProj;
        Matrix preView;
        Vector3 camPos;
        float iTime;
        Vector3 camFront;
        float upTime;
        float delta;
    } uniformBuffer;
    struct
    {
        bool alt = false;
        bool left = false;
        bool mid = false;
        bool right = false;
        Vector2 pos;
    } mouse;

    D3D12_CONSTANT_BUFFER_VIEW_DESC bufDes;
    void Init(SkAgent *initAgent)
    {
        // base = initAgent;
        agent = initAgent;
        base = agent->GetBase();
        cam = &base->cam;
        Vector3 eyePos{0.0f, 3.0f, 10.0f};
        Vector3 focusPos{0.0f, 0.0f, 0.0f};
        Vector3 up{0.0f, 1.0f, 0.0f};
        // TODO:
        Vector3 right = DirectX::XMVector3Cross(eyePos - focusPos, up);
        up = DirectX::XMVector3Cross(right, eyePos - focusPos);
        cam->SetLens(DirectX::XMConvertToRadians(45.0f), (float)base->width / base->height, 0.01f, 100.0f);
        cam->SetLookAt(eyePos, Vector3(0.0f), up);
        cam->UpdateView();
        // Show(cam->proj);
        // uniformBuffer.projection = Matrix::CreatePerspectiveFieldOfView(DirectX::XMConvertToRadians(90.0f), (float)base->width / base->height, 0.01f, 20.0f);
        // uniformBuffer.view = Matrix::CreateLookAt(eyePos, Vector3(0.0f), up);
        uniformBuffer.projection = cam->proj.Transpose();
        uniformBuffer.view = cam->view.Transpose();
        uniformBuffer.jitterProj = uniformBuffer.projection;
        uniformBuffer.preProj = uniformBuffer.projection;
        uniformBuffer.preView = uniformBuffer.view;
        uniformBuffer.camPos = cam->pos;
        uniformBuffer.camFront = cam->front;
        uniformBuffer.delta=0.016f;
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
    float RadicalInverse(uint32_t Base, uint64_t i)
    {
        float Digit, Radical, Inverse;
        Digit = Radical = 1.0f / (float)Base;
        Inverse = 0.0f;
        while (i)
        {
            // i余Base求出i在"Base"进制下的最低位的数
            // 乘以Digit将这个数镜像到小数点右边
            Inverse += Digit * (float)(i % Base);
            Digit *= Radical;

            // i除以Base即可求右一位的数
            i /= Base;
        }
        return Inverse;
    }
    void Update()
    {
        cam->Walking(base->delta);
        MouseProc();
        cam->UpdateView();
        uniformBuffer.preProj = uniformBuffer.jitterProj;
        uniformBuffer.preView = uniformBuffer.view;
        uniformBuffer.projection = cam->proj.Transpose();
        uniformBuffer.view = cam->view.Transpose();
        uniformBuffer.jitterProj = uniformBuffer.projection;
        uniformBuffer.jitterProj.m[0][2] += (1.0f - 2.0f * RadicalInverse(5, base->iFrame)) / base->width;
        uniformBuffer.jitterProj.m[1][2] += (1.0f - 2.0f * RadicalInverse(3, base->iFrame)) / base->height;
        uniformBuffer.camPos = cam->pos;
        uniformBuffer.camFront = cam->front;
        uniformBuffer.iTime = base->timer;
        uniformBuffer.upTime = cam->upTime;
        uniformBuffer.delta=base->delta;
        SK_CHECK(uniBuf.Map());
        memcpy(uniBuf.data, &uniformBuffer, uniBuf.bufSize);
        uniBuf.Unmap();
    }
    void MouseProc()
    {
        POINT point;
        GetCursorPos(&point);
        float x = (float)(point.x) / base->width;
        float y = (float)(point.y) / base->height;
        float dx = mouse.pos.x - x;
        float dy = mouse.pos.y - y;
        if (dx != 0.0f || dy != 0.0f)
        {
            if (mouse.left)
            {
                if (mouse.alt)
                {
                    cam->Turn(dx, dy);
                }
                else
                {
                    cam->FocusOn(dx, dy);
                }
            }
            else if (mouse.mid)
            {
                cam->Lift(-dy * 10.0f);
                cam->Strafe(dx * 10.0f);
            }
        }

        mouse.pos.x = x;
        mouse.pos.y = y;
    }
    void WinProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override
    {
        switch (uMsg)
        {
        case WM_SYSKEYDOWN:
            mouse.left = true;
            break;
        case WM_SYSKEYUP:
            mouse.left = false;
            break;
        case WM_KEYDOWN:
            Event_Key(wParam, true);
            break;
        case WM_KEYUP:
            Event_Key(wParam, false);
            break;
        case WM_LBUTTONDOWN:
            Event_LButton(wParam, lParam, true);
            break;
        case WM_LBUTTONUP:
            Event_LButton(wParam, lParam, false);
            break;
        case WM_MBUTTONDOWN:
            Event_MButton(wParam, lParam, true);
            break;
        case WM_MBUTTONUP:
            Event_MButton(wParam, lParam, false);
            break;
        case WM_MOUSEMOVE:
            break;
        case WM_MOUSELEAVE:
            break;
        case WM_MOUSEWHEEL:
            Event_Wheel(wParam);
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
        case 'E':
        case 'e':
            cam->keys.up = isDown;
            break;
        case 'F':
        case 'f':
            cam->keys.down = isDown;
            break;
        default:
            break;
        }
    }
    void Event_LButton(WPARAM wParam, LPARAM lParam, bool isDown)
    {
        mouse.left = isDown;
    }
    void Event_MButton(WPARAM wParam, LPARAM lParam, bool isDown)
    {
        mouse.mid = isDown;
    }
    void Event_Wheel(LPARAM wParam)
    {
        int pos = (int)(short)HIWORD(wParam) / 120;
        cam->Zoom((float)(pos));
    }
};
