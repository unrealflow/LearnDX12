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
SkPass pass;
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
    pass.Init(base);

    CD3DX12_DESCRIPTOR_RANGE1 ranges[1];
    ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);

    std::vector<CD3DX12_ROOT_PARAMETER1> rootParameters{2};
    rootParameters[0].InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_PIXEL);
    rootParameters[1].InitAsConstantBufferView(0);
    
    tex.Setup();
    con.Setup();

    pass.CreateRoot(rootParameters);
    pass.CreatePipeline(model.inputDescs, L"shader/shader.hlsl");
    pass.Add(0,base->srvHeap.Get());
    pass.Add(1,con.uniBuf.buf->GetGPUVirtualAddress());
    cmd.AddPass(&pass);
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
