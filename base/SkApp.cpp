#include "SkApp.h"
#include "SkDevice.h"
#include "SkCmd.h"
#include "SkTex.h"
#include "SkPass.h"
#include "SkModel.h"
#include "SkController.h"
#include "SkAgent.h"
#include "SkTarget.h"
#include "SkComputer.h"
#include "SkAA.h"

//srv [0]
SkTex tex;

//srv [1]
SkTex bk;
SkModel model;
SkPass p_gbuffer;
SkPass p_deferred;
SkPass p_post0;
SkPass p_post1;
SkPass p_AO;
SkComputer p_blur;

//rtv [3,4,5]
//srv [2,3,4]
SkGBufferRT rt_gbuffer;
//rtv [6]
//srv [5]
SkImageRT rt_AO;
//rtv [7]
//srv [6]
SkImageRT rt_deferred;
//rtv [8]
//srv [7]
SkImageRT rt_post0;
SkDefaultRT rt_post1;
//srv[8,9,10]
SkAA p_aa;
void SkApp::Setup()
{

    con.Setup();
    // tex.InitCheckerboard();
    {
        //srv [0]
        tex.Init(&agent, GetAssetFullPath("texture/pic.jpg"));
        tex.Setup(0);
    }
    {
        model.Init(&agent);
        // model.ImportModel(GetAssetFullPath("model/vk.obj"));
        model.ImportModel(GetAssetFullPath("model/model.obj"));

        model.mesh.Setup();
        model.matSet.Setup();
    }
    {
        //srv [1]
        bk.Init(&agent, GetAssetFullPath("texture/scene.jpg"), 1);
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
        std::vector<CD3DX12_ROOT_PARAMETER1> rootParameters{4};
        rootParameters[0].InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_PIXEL);
        rootParameters[1].InitAsConstantBufferView(0);
        rootParameters[2].InitAsConstantBufferView(1);
        rootParameters[3].InitAsConstantBufferView(2);

        p_gbuffer.CreateRoot(&rootParameters, &rt_gbuffer);
        p_gbuffer.AddMesh(&model.mesh);
        p_gbuffer.CreatePipeline(model.inputDescs, L"shader/GBuffer.hlsl");
        p_gbuffer.AddDesc(0, base->heap->GetHeapSRV());
        p_gbuffer.AddDesc(1, con.uniBuf.buf->GetGPUVirtualAddress());
        p_gbuffer.AddDesc(2, model.matSet.infoBuf.buf->GetGPUVirtualAddress());
        p_gbuffer.AddDesc(3, model.matSet.matBuf.buf->GetGPUVirtualAddress());

        cmd.AddPass(&p_gbuffer);
    }
    {

        rt_AO.Init(base);
        rt_AO.Setup(base->heap, 6, 5, false);

        // p_AO.Init(base);
        // CD3DX12_DESCRIPTOR_RANGE1 ranges[1];
        // ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 3, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_VOLATILE);
        // std::vector<CD3DX12_ROOT_PARAMETER1> rootParameters{2};
        // rootParameters[0].InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_PIXEL);
        // rootParameters[1].InitAsConstantBufferView(0);

        // p_AO.CreateRoot(&rootParameters, &rt_AO);
        // p_AO.CreatePipeline(model.inputDescs, L"shader/AO.hlsl");
        // p_AO.AddDesc(0, base->heap->GetHeapSRV(), 2);
        // p_AO.AddDesc(1, con.uniBuf.buf->GetGPUVirtualAddress());

        // cmd.AddPass(&p_AO);
    }
    {
        // p_blur.Init(base);

        // p_blur.CreateRoot(nullptr);
        // p_blur.CreatePipeline(L"shader/Blur.hlsl");
        // // p_blur.CreateInputView(rt_gbuffer.albedo.Get(),rt_gbuffer.GetFormat(0));
        // p_blur.CreateInputView(rt_AO.texture.Get(), rt_AO.GetFormat(0));
        // // p_blur.CreateOutputView(rt_AO.texture.Get(),rt_AO.GetFormat(0));
        // p_blur.UseDefaultTexture(rt_AO.GetFormat(0));

        // cmd.AddPass(&p_blur);
    }
    {
        //input : tex,bk ,GBuffer,AO
        //output: imageRT
        rt_deferred.Init(base);
        rt_deferred.Setup(base->heap, 7, 6, false);

        p_deferred.Init(base);
        CD3DX12_DESCRIPTOR_RANGE1 ranges[2];
        ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 6, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_VOLATILE);
        ranges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 3, 6, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_VOLATILE);


        std::vector<CD3DX12_ROOT_PARAMETER1> rootParameters{4};
        rootParameters[0].InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_PIXEL);
        rootParameters[1].InitAsConstantBufferView(0);
        rootParameters[2].InitAsConstantBufferView(1);
        rootParameters[3].InitAsDescriptorTable(1, &ranges[1], D3D12_SHADER_VISIBILITY_PIXEL);


        p_deferred.CreateRoot(&rootParameters, &rt_deferred);
        p_deferred.CreatePipeline(model.inputDescs, L"shader/Deferred.hlsl");
        p_deferred.AddDesc(0, base->heap->GetHeapSRV());
        p_deferred.AddDesc(1, con.uniBuf.buf->GetGPUVirtualAddress());
        p_deferred.AddDesc(2, model.matSet.matBuf.buf->GetGPUVirtualAddress());
        p_deferred.AddDesc(3, base->heap->GetHeapSRV(),8,false);

        cmd.AddPass(&p_deferred);
    }
    {
        //input :GBufferRT,AO,ImageRT
        //output :SwapChianImage
        rt_post0.Init(base);
        rt_post0.Setup(base->heap, 8, 7, false);

        CD3DX12_DESCRIPTOR_RANGE1 ranges[2];
        ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 5, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_VOLATILE);
        ranges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 3, 5, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_VOLATILE);

        std::vector<CD3DX12_ROOT_PARAMETER1> rootParameters{3};
        rootParameters[0].InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_PIXEL);
        rootParameters[1].InitAsConstantBufferView(0);
        rootParameters[2].InitAsDescriptorTable(1, &ranges[1], D3D12_SHADER_VISIBILITY_PIXEL);
        
        // rt_post1.Init(base);
        p_post0.Init(base);
        p_post0.CreateRoot(&rootParameters, &rt_post0);
        p_post0.CreatePipeline(model.inputDescs, L"shader/Post0.hlsl");
        p_post0.AddDesc(0, base->heap->GetHeapSRV(), 2);
        p_post0.AddDesc(1, con.uniBuf.buf->GetGPUVirtualAddress());
        p_post0.AddDesc(2, base->heap->GetHeapSRV(), 8, false);

        cmd.AddPass(&p_post0);
    }
    {
        rt_post1.Init(base);

        CD3DX12_DESCRIPTOR_RANGE1 ranges[2];
        ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 5, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_VOLATILE);
        ranges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 5, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_VOLATILE);

        std::vector<CD3DX12_ROOT_PARAMETER1> rootParameters{3};
        rootParameters[0].InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_PIXEL);
        rootParameters[1].InitAsConstantBufferView(0);
        rootParameters[2].InitAsDescriptorTable(1, &ranges[1], D3D12_SHADER_VISIBILITY_PIXEL);
        
        
        p_post1.Init(base);
        p_post1.CreateRoot(&rootParameters, &rt_post1);
        p_post1.CreatePipeline(model.inputDescs, L"shader/Post1.hlsl");
        p_post1.AddDesc(0, base->heap->GetHeapSRV(), 2);
        p_post1.AddDesc(1, con.uniBuf.buf->GetGPUVirtualAddress());
        p_post1.AddDesc(2, base->heap->GetHeapSRV(), 7, false);

        cmd.AddPass(&p_post1);
      
    }
    {
        p_aa.Init(base, &rt_gbuffer, &rt_post0);
        p_aa.Setup(base->heap, 8);
        cmd.AddPass(&p_aa);
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
