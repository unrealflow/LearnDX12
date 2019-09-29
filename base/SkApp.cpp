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
SkPass p_gbuffer;
SkPass p_deferred;
SkPass p_post;
SkPass p_AO;

//rtv [3,4,5]
//srv [2,3,4]
SkGBufferRT rt_gbuffer;
//rtv [6]
//srv [4]
SkImageRT rt_deferred;
SkDefaultRT rt_post;
SkImageRT rt_AO;
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
        bk.Init(&agent, GetAssetFullPath("texture/scene.jpg"), 8);
        bk.Setup(1);
    }
    {
        //Input : tex
        //Output : GBufferRT
        rt_gbuffer.Init(base);
        rt_gbuffer.Setup(base->heap, 3, 2);

        p_gbuffer.Init(base);
        CD3DX12_DESCRIPTOR_RANGE1 ranges[1];
        ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);
        std::vector<CD3DX12_ROOT_PARAMETER1> rootParameters{2};
        rootParameters[0].InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_PIXEL);
        rootParameters[1].InitAsConstantBufferView(0);

        p_gbuffer.CreateRoot(rootParameters, &rt_gbuffer);
        p_gbuffer.AddMesh(&model.mesh);
        p_gbuffer.CreatePipeline(model.inputDescs, L"shader/GBuffer.hlsl");
        p_gbuffer.AddDesc(0, base->heap->GetHeapSRV());
        p_gbuffer.AddDesc(1, con.uniBuf.buf->GetGPUVirtualAddress());

        cmd.AddPass(&p_gbuffer);
    }
    {

        rt_AO.Init(base);
        rt_AO.Setup(base->heap, 6, 5);

        p_AO.Init(base);
        CD3DX12_DESCRIPTOR_RANGE1 ranges[1];
        ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 3, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_VOLATILE);
        std::vector<CD3DX12_ROOT_PARAMETER1> rootParameters{2};
        rootParameters[0].InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_PIXEL);
        rootParameters[1].InitAsConstantBufferView(0);

        
        p_AO.CreateRoot(rootParameters,&rt_AO);
        p_AO.CreatePipeline(model.inputDescs, L"shader/AO.hlsl");
        p_AO.AddDesc(0,base->heap->GetHeapSRV(),2);
        p_AO.AddDesc(1, con.uniBuf.buf->GetGPUVirtualAddress());

        cmd.AddPass(&p_AO);
    }
    {
        //input : tex,bk ,GBuffer,AO
        //output: imageRT
        rt_deferred.Init(base);
        rt_deferred.Setup(base->heap, 7, 6);

        p_deferred.Init(base);
        CD3DX12_DESCRIPTOR_RANGE1 ranges[1];
        ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 6, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_VOLATILE);

        std::vector<CD3DX12_ROOT_PARAMETER1> rootParameters{2};
        rootParameters[0].InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_PIXEL);
        rootParameters[1].InitAsConstantBufferView(0);

        p_deferred.CreateRoot(rootParameters, &rt_deferred);
        p_deferred.CreatePipeline(model.inputDescs, L"shader/Deferred.hlsl");
        p_deferred.AddDesc(0, base->heap->GetHeapSRV());
        p_deferred.AddDesc(1, con.uniBuf.buf->GetGPUVirtualAddress());
        cmd.AddPass(&p_deferred);
    }
    {
        //input :GBufferRT,AO,ImageRT
        //output :SwapChianImage
        p_post.Init(base);
        CD3DX12_DESCRIPTOR_RANGE1 ranges[1];
        ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 5, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_VOLATILE);

        std::vector<CD3DX12_ROOT_PARAMETER1> rootParameters{2};
        rootParameters[0].InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_PIXEL);
        rootParameters[1].InitAsConstantBufferView(0);

        rt_post.Init(base);
        p_post.CreateRoot(rootParameters, &rt_post);
        p_post.CreatePipeline(model.inputDescs, L"shader/Post.hlsl");
        p_post.AddDesc(0, base->heap->GetHeapSRV(), 2);
        p_post.AddDesc(1, con.uniBuf.buf->GetGPUVirtualAddress());
        cmd.AddPass(&p_post);
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
