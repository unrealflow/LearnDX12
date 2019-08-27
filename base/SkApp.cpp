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
void SkApp::Setup()
{
    // mesh.Init(&agent);
    // std::vector<D3D12_INPUT_ELEMENT_DESC> inputDescs;
    // mesh.SetupTriangle(inputDescs);
    model.Init(&agent);
    model.ImportModel(GetAssetFullPath("model/vk.obj"));
    tex.Init(&agent,GetAssetFullPath("texture/pic.jpg"));
    model.mesh.Setup();
    // tex.InitCheckerboard();
    pipeline.Setup(model.inputDescs);
    // pipeline.Setup(inputDescs);
    tex.Setup(); 
    con.Setup();
    cmd.AddConstBuf(1,&con.uniBuf);
    cmd.AddMesh(&model.mesh);
    // cmd.AddMesh(&mesh);
    cmd.BuildCmdLists();
}
void SkApp::Draw()
{
    con.Update();
    cmd.Submit();
}
