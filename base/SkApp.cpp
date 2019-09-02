#include "SkApp.h"
#include "SkPipeline.h"
#include "SkCmd.h"
#include "SkTex.h"
#include "SkModel.h"
#include "SkController.h"
#include "SkAgent.h"
SkMesh mesh;
SkTex tex;
SkModel model;
SkPass pass0;
SkPass pass1;
SkDefaultRT rt;
SkImageRT rt2;
void SkApp::Setup()
{
    // mesh.Init(&agent);
    // std::vector<D3D12_INPUT_ELEMENT_DESC> inputDescs;
    // mesh.SetupTriangle(inputDescs);
    // model.Init(&agent);
    // model.ImportModel(GetAssetFullPath("model/vk.obj"));
    tex.Init(&agent, GetAssetFullPath("texture/pic.jpg"));
    // model.mesh.Setup();
    // tex.InitCheckerboard();

    tex.Setup();
    con.Setup();

    {
        pass0.Init(base);

        CD3DX12_DESCRIPTOR_RANGE1 ranges[1];
        ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);

        std::vector<CD3DX12_ROOT_PARAMETER1> rootParameters{2};
        rootParameters[0].InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_PIXEL);
        rootParameters[1].InitAsConstantBufferView(0);

        rt2.Init(base);
        rt2.CreateResource();
        CD3DX12_CPU_DESCRIPTOR_HANDLE srv{base->srvHeap->GetCPUDescriptorHandleForHeapStart(), 1, base->srvDesSize};
        CD3DX12_CPU_DESCRIPTOR_HANDLE rtv{base->rtvHeap->GetCPUDescriptorHandleForHeapStart(), (int)base->imageCount, base->rtvDesSize};
        rt2.CreateView(srv, rtv);

        pass0.CreateRoot(rootParameters, &rt2);
        pass0.CreatePipeline(model.inputDescs, L"shader/pass0.hlsl");
        pass0.AddDesc(0, base->srvHeap.Get());
        pass0.AddDesc(1, con.uniBuf.buf->GetGPUVirtualAddress());
        cmd.AddPass(&pass0);
    }

    {
        pass1.Init(base);

        CD3DX12_DESCRIPTOR_RANGE1 ranges[1];
        ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 2, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);

        std::vector<CD3DX12_ROOT_PARAMETER1> rootParameters{2};
        rootParameters[0].InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_PIXEL);
        rootParameters[1].InitAsConstantBufferView(0);

        rt.Init(base);

        pass1.CreateRoot(rootParameters, &rt);
        pass1.CreatePipeline(model.inputDescs, L"shader/pass1.hlsl");
        pass1.AddDesc(0, base->srvHeap.Get());
        pass1.AddDesc(1, con.uniBuf.buf->GetGPUVirtualAddress());
        cmd.AddPass(&pass1);
    }
    // cmd.AddMesh(&model.mesh);
    // cmd.AddMesh(&mesh);
    cmd.BuildCmdLists();
}
void SkApp::Draw()
{
    con.Update();
    cmd.Submit();
    cmd.Present();
}
