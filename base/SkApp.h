#pragma once
#include "SkBase.h"
#include "SkWin32.h"
#include "SkDevice.h"
#include "SkCmd.h"
#include "SkMesh.h"
#include "SkController.h"
class SkApp
{
protected:
    SkBase *base = nullptr;
    SkWin32 win;
    SkDevice device;
    SkCmd cmd;
    SkAgent agent;
    SkController con;
    void Init()
    {
        win.Init(base);
        win.InitWindow();
        device.Init(base);
        agent.Init(base);
        cmd.Init(base);
        con.Init(&agent);
        Setup();
        win.Register(&con);
    }
    void Setup();
    void Draw();
    void Loop()
    {
        MSG msg = {};
        float lastTime = GetMilliTime();
        while (msg.message != WM_QUIT)
        {
            base->timer = GetMilliTime();
            base->delta = base->timer - lastTime;
            lastTime = base->timer;
            Draw();
            // Process any messages in the queue.
            if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
    }
    void CleanUp()
    {
        cmd.CleanUp();
    }

public:
    SkApp(uint32_t width, uint32_t height, std::string winName)
    {
        base = new SkBase();
        base->width = width;
        base->height = height;
        base->name = winName;
        base->imageIndex = 0;
        // base->rtvDesSize = 0;
        // base->srvDesSize = 0;
        // base->dsvDesSize = 0;
    }
    ~SkApp()
    {
        delete base;
        base = nullptr;
    }
    void Run()
    {
        fprintf(stderr, "SkApp::Run...\n");
        Init();
        Loop();
        CleanUp();
    }
};
