#pragma once
#include "SkBase.h"
#include "SkMesh.h"
#include "SkFence.h"
#include "SkPass.h"
#include "SkTarget.h"
class SkCmd
{
private:
    SkBase *base;
    std::vector<ComPtr<ID3D12GraphicsCommandList4>> cmdLists;
    std::vector<SkFence> fences;
    // std::unordered_map<uint32_t,std::vector<SkMesh *>> meshes;
    std::vector<ISkPass *> passes;

    void CreateFence()
    {
        fences.resize(base->imageCount);
        for (size_t i = 0; i < fences.size(); i++)
        {
            fences[i].Create(base);
        }
    }
    void LoadAssets()
    {
    }

public:
    void Init(SkBase *initBase)
    {
        base = initBase;
        // meshes.clear();
    }
    void AddMesh(SkMesh *mesh, uint32_t p = 0)
    {
        // meshes[p].emplace_back(mesh);
    }
    void AddPass(ISkPass *pass)
    {
        this->passes.push_back(pass);
    }
    void BuildCmdLists()
    {
        fprintf(stderr, "Cmd::BuildCmdLists...\n");
        LoadAssets();
        CreateFence();
        // Command list allocators can only be reset when the associated
        // command lists have finished execution on the GPU; apps should use
        // fences to determine GPU execution progress.
        this->cmdLists.resize(base->imageCount);
        for (size_t i = 0; i < this->cmdLists.size(); i++)
        {
            SK_CHECK(base->device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, base->cmdPool.Get(), nullptr, IID_PPV_ARGS(&this->cmdLists[i])));
            this->cmdLists[i]->Close();
        }
        // SK_CHECK(base->cmdPool->Reset());
        for (int i = 0; i < this->cmdLists.size(); i++)
        {
            // However, when ExecuteCommandList() is called on a particular command
            // list, that command list can then be reset at any time and must be before
            // re-recording.
            if (this->cmdLists[i].Get() == nullptr)
            {
                fprintf(stderr, "Nullptr...\n");
            }
            this->cmdLists[i]->Reset(base->cmdPool.Get(), nullptr);
            for (uint32_t p = 0; p < passes.size(); p++)
            {
                fprintf(stderr,"passes[%d]:...\n",p);
                
                passes[p]->CmdSet(this->cmdLists[i], i);
            }

            SK_CHECK(this->cmdLists[i]->Close());
        }
        fprintf(stderr, "Cmd::BuildCmdLists...OK\n");
    }
    void Submit()
    {
        fences[base->imageIndex].Wait();
        ID3D12CommandList *ppCommandLists[] = {this->cmdLists[base->imageIndex].Get()};
        base->cmdQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
    }
    void Present()
    {
        // Present the frame.
        SK_CHECK(base->swapChain->Present(1, 0));
        fences[base->imageIndex].Signal();
        base->imageIndex = base->swapChain->GetCurrentBackBufferIndex();
    }
    void CleanUp()
    {
        // Ensure that the GPU is no longer referencing resources that are about to be
        // cleaned up by the destructor.
        SkFence::CleanAllFence(fences);
        // meshes.clear();
    }
    void BeginCmd(ComPtr<ID3D12GraphicsCommandList> &cmd)
    {
        SK_CHECK(base->device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, base->cmdPool.Get(), nullptr, IID_PPV_ARGS(&cmd)));
    }
    void FlushCmd(ComPtr<ID3D12GraphicsCommandList> &cmd)
    {
        SK_CHECK(cmd->Close());
        ID3D12CommandList *ppCommandLists[] = {cmd.Get()};
        base->cmdQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
    }
};
