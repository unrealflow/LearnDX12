﻿#pragma once
#include "SkBase.h"
#include "SkWin32.h"
#include "SkPipeline.h"
#include "SkCmd.h"
class SkApp
{
protected:
    SkBase *base = nullptr;
    SkWin32 win;
    SkPipeline pipeline;
    SkCmd cmd;
    void Init()
    {
        win.Init(base);
        win.InitWindow();
        pipeline.Init(base);
        pipeline.Setup();
        cmd.Init(base);
        cmd.BuildCmdLists();
    } 
    void Loop()
    {
        MSG msg = {};
        while (msg.message != WM_QUIT)
        {
            cmd.Submit();
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
    SkApp(uint32_t width, uint32_t height, string winName)
    {
        base = new SkBase();
        base->width = width;
        base->height = height;
        base->name = winName;
        base->imageIndex = 0;
        base->desSize = 0;
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
