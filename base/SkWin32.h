#pragma once
#include "SkBase.h"
class SkWin32;
static SkWin32 *gWin32 = nullptr;
class SkWin32
{
private:
    SkBase *base;
    std::vector<SkCallback *> callbacks;
    void WinProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        for (auto &&c : callbacks)
        {
            c->WinProc(hWnd, uMsg, wParam, lParam);
        }
    }

public:
    SkWin32()
    {
        gWin32 = this;
    }
    void Init(SkBase *initBase)
    {
        base = initBase;
        callbacks.clear();
    }
    void Register(SkCallback *call)
    {
        callbacks.push_back(call);
    }

    void InitWindow()
    {
        base->hInstance = GetModuleHandle(NULL);
        base->iCmdShow = SW_SHOWNORMAL;
        WNDCLASSEX windowClass = {};
        windowClass.cbSize = sizeof(WNDCLASSEX);
        windowClass.style = CS_HREDRAW | CS_VREDRAW;
        windowClass.lpfnWndProc = StaticWinProc;
        windowClass.hInstance = base->hInstance;
        windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
        windowClass.lpszClassName = LPCSTR(L"DXSampleClass");
        RegisterClassEx(&windowClass);

        RECT windowRect = {0, 0, static_cast<LONG>(base->width), static_cast<LONG>(base->height)};
        AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);

        // Create the window and store a handle to it.
        base->hwnd = CreateWindow(
            windowClass.lpszClassName,
            base->name.c_str(),
            WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            windowRect.right - windowRect.left,
            windowRect.bottom - windowRect.top,
            nullptr, // We have no parent window.
            nullptr, // We aren't using menus.
            base->hInstance,
            0);

        // Initialize the sample. OnInit is defined in each child-implementation of DXSample.

        ShowWindow(base->hwnd, base->iCmdShow);

        // Main sample loop.
    }

    static LRESULT CALLBACK StaticWinProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        if (nullptr != gWin32)
        {
            gWin32->WinProc(hWnd, uMsg, wParam, lParam);
        }
        switch (uMsg)
        {
        case WM_CLOSE:
        case WM_QUIT:
            // prepared = false;
            DestroyWindow(hWnd);
            PostQuitMessage(0);
            return 0;
        case WM_CREATE:
        {
            // Save the DXSample* passed in to CreateWindow.
            LPCREATESTRUCT pCreateStruct = reinterpret_cast<LPCREATESTRUCT>(lParam);
            SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pCreateStruct->lpCreateParams));
        }
            return 0;
        case WM_KEYDOWN:
            if (wParam == VK_ESCAPE)
            {
                PostMessage(hWnd,WM_QUIT, 0, 0);
            }
            return 0;
        case WM_KEYUP:
            return 0;
        case WM_PAINT:
            return 0;
        case WM_DESTROY:
            PostMessage(hWnd,WM_QUIT, 0, 0);
            return 0;
        case WM_LBUTTONDOWN:
            return 0;
        case WM_LBUTTONUP:
            return 0;
        default:
            break;
        }
        return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }
};
