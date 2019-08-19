#include "SkApp.h"
#include "SkPipeline.h"
#include "SkCmd.h"
#include "SkTex.h"
#include "SkModel.h"
#include "SkController.h"
// SkMesh mesh;
SkTex tex;
SkModel model;
SkController con;
void SkApp::Setup()
{
    // mesh.Init(this->base);
    // std::vector<D3D12_INPUT_ELEMENT_DESC> inputDescs;
    // mesh.SetupTriangle(inputDescs);
    model.Init(this->base);
    model.ImportModel(GetAssetFullPath("model/vk.obj"));
    tex.Init(base,GetAssetFullPath("texture/pic.jpg"));
    model.mesh.Setup();
    con.Init(base);
   
    // tex.InitCheckerboard();
    pipeline.Setup(model.inputDescs);
    tex.Setup(); 
    con.Setup();
    cmd.AddConstBuf(1,&con.uniBuf);
    cmd.AddMesh(&model.mesh);
    cmd.BuildCmdLists();
}
