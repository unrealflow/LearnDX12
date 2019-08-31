#pragma once
#include "SkBase.h"
#include "SkMesh.h"
#include "SkFence.h"
#include "SkPass.h"
class SkCmd
{
private:
    SkBase *base;
    std::vector<ComPtr<ID3D12GraphicsCommandList>> cmdLists;
    std::vector<SkMesh *> meshes;
    std::vector<std::pair<uint32_t, SkBuffer *>> buffers;
    std::vector<SkFence> fences;
    std::vector<SkPass *> passes;

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
        meshes.clear();
        buffers.clear();
    }
    void AddMesh(SkMesh *mesh)
    {
        meshes.emplace_back(mesh);
    }
    void AddPass(SkPass *pass)
    {
        this->passes.push_back(pass);
    }
    void AddConstBuf(uint32_t paramIndex, SkBuffer *buf)
    {
        buffers.emplace_back(std::pair<uint32_t, SkBuffer *>(paramIndex, buf));
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
        CD3DX12_VIEWPORT m_viewport{0.0f, 0.0f, static_cast<float>(base->width), static_cast<float>(base->height)};
        CD3DX12_RECT m_scissorRect{0, 0, static_cast<LONG>(base->width), static_cast<LONG>(base->height)};

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
            for (size_t p = 0; p < passes.size(); p++)
            {
                passes[p]->CmdSet(this->cmdLists[i]);
                this->cmdLists[i]->RSSetViewports(1, &m_viewport);
                this->cmdLists[i]->RSSetScissorRects(1, &m_scissorRect);

                // Indicate that the back buffer will be used as a render target.
                this->cmdLists[i]->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(base->renderTargets[i].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

                CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(base->rtvHeap->GetCPUDescriptorHandleForHeapStart(), i, base->rtvDesSize);
                this->cmdLists[i]->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);
                // Record commands.
                const float clearColor[] = {0.0f, 0.1f, 0.1f, 1.0f};
                this->cmdLists[i]->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

                for (size_t mc = 0; mc < meshes.size(); mc++)
                {
                    meshes[mc]->Draw(this->cmdLists[i].Get());
                }
                if (0 == meshes.size())
                {
                    this->cmdLists[i]->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
                    this->cmdLists[i]->DrawInstanced(3, 1, 0, 0);
                }
                // Indicate that the back buffer will now be used to present.
                this->cmdLists[i]->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(base->renderTargets[i].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));
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
        meshes.clear();
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
