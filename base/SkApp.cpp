#include "SkApp.h"
#include "SkDevice.h"
#include "SkCmd.h"
#include "SkTex.h"
#include "SkPass.h"
#include "SkModel.h"
#include "SkController.h"
#include "SkAgent.h"

SkTex tex;
SkTex bk;
SkModel model;
SkPass pass0;
SkPass pass1;
SkPass pass2;
//rtv [3,4,5]
//srv [2,3,4]
SkGBufferRT rt0;
//rtv [6]
//srv [4]
SkImageRT rt1;
SkDefaultRT rt2;
void SkApp::Setup()
{

    model.Init(&agent);
    model.ImportModel(GetAssetFullPath("model/vk.obj"));

    model.mesh.Setup();
    con.Setup();
    // tex.InitCheckerboard();
    {
        //srv [0]
        tex.Init(&agent, GetAssetFullPath("texture/pic.jpg"));
        tex.Setup(0);
    }
    {
        //srv [1]
        bk.Init(&agent, GetAssetFullPath("texture/scene.jpg"),3);
        bk.Setup(1);
    }
    {
        //Input : tex
        //Output : GBufferRT
        pass0.Init(base);

        CD3DX12_DESCRIPTOR_RANGE1 ranges[1];
        ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);

        std::vector<CD3DX12_ROOT_PARAMETER1> rootParameters{2};
        rootParameters[0].InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_PIXEL);
        rootParameters[1].InitAsConstantBufferView(0);

        rt0.Init(base);
        rt0.Setup(base->heap, 3, 2);
        pass0.CreateRoot(rootParameters, &rt0);
        pass0.AddMesh(&model.mesh);
        // pass0.CreatePipeline(model.inputDescs, L"shader/pass0.hlsl");
        pass0.CreatePipeline(model.inputDescs, L"shader/shader.hlsl");
        pass0.AddDesc(0, base->heap->GetHeapSRV());
        pass0.AddDesc(1, con.uniBuf.buf->GetGPUVirtualAddress());

        cmd.AddPass(&pass0);
    }
    {
        //input : tex,bk ,GBuffer
        //output: imageRT
        pass1.Init(base);
        CD3DX12_DESCRIPTOR_RANGE1 ranges[1];
        ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 5, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_VOLATILE);

        std::vector<CD3DX12_ROOT_PARAMETER1> rootParameters{2};
        rootParameters[0].InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_PIXEL);
        rootParameters[1].InitAsConstantBufferView(0);

        rt1.Init(base);
        rt1.Setup(base->heap, 6, 5);

        pass1.CreateRoot(rootParameters, &rt1);
        pass1.CreatePipeline(model.inputDescs, L"shader/pass1.hlsl");
        pass1.AddDesc(0, base->heap->GetHeapSRV());
        pass1.AddDesc(1, con.uniBuf.buf->GetGPUVirtualAddress());
        cmd.AddPass(&pass1);
    }
    {
        //input :GBufferRT,ImageRT
        //output :SwapChianImage
        pass2.Init(base);
        CD3DX12_DESCRIPTOR_RANGE1 ranges[1];
        ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 4, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_VOLATILE);

        std::vector<CD3DX12_ROOT_PARAMETER1> rootParameters{2};
        rootParameters[0].InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_PIXEL);
        rootParameters[1].InitAsConstantBufferView(0);

        rt2.Init(base);
        pass2.CreateRoot(rootParameters, &rt2);
        pass2.CreatePipeline(model.inputDescs, L"shader/pass2.hlsl");
        pass2.AddDesc(0, base->heap->GetHeapSRV(), 2);
        pass2.AddDesc(1, con.uniBuf.buf->GetGPUVirtualAddress());
        cmd.AddPass(&pass2);
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
