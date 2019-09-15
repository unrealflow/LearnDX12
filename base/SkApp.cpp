#include "SkApp.h"
#include "SkDevice.h"
#include "SkCmd.h"
#include "SkTex.h"
#include "SkPass.h"
#include "SkModel.h"
#include "SkController.h"
#include "SkAgent.h"

SkTex tex;
SkModel model;
SkPass pass0;
SkPass pass1;
SkGBufferRT rt0;
SkDefaultRT rt1;
void SkApp::Setup()
{

    model.Init(&agent);
    model.ImportModel(GetAssetFullPath("model/vk.obj"));

    model.mesh.Setup();
    // tex.InitCheckerboard();
    {
        tex.Init(&agent, GetAssetFullPath("texture/pic.jpg"));
        tex.Setup(0);
        con.Setup();
    }

    {
        pass0.Init(base);

        CD3DX12_DESCRIPTOR_RANGE1 ranges[1];
        ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);

        std::vector<CD3DX12_ROOT_PARAMETER1> rootParameters{2};
        rootParameters[0].InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_PIXEL);
        rootParameters[1].InitAsConstantBufferView(0);

        rt0.Init(base);
        rt0.Setup(base->heap,3,1);
        pass0.CreateRoot(rootParameters, &rt0);
        pass0.AddMesh(&model.mesh);
        // pass0.CreatePipeline(model.inputDescs, L"shader/pass0.hlsl");
        pass0.CreatePipeline(model.inputDescs, L"shader/shader.hlsl");
        pass0.AddDesc(0, base->heap->GetHeapSRV());
        pass0.AddDesc(1, con.uniBuf.buf->GetGPUVirtualAddress());
       
        cmd.AddPass(&pass0);
    }

    {
        pass1.Init(base);

        CD3DX12_DESCRIPTOR_RANGE1 ranges[1];
        ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 4, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_VOLATILE);

        std::vector<CD3DX12_ROOT_PARAMETER1> rootParameters{2};
        rootParameters[0].InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_PIXEL);
        rootParameters[1].InitAsConstantBufferView(0);


        rt1.Init(base);

        pass1.CreateRoot(rootParameters, &rt1);
        pass1.CreatePipeline(model.inputDescs, L"shader/pass1.hlsl");
        pass1.AddDesc(0, base->heap->GetHeapSRV());
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
