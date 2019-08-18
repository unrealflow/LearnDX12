#include "SkApp.h"
#include "SkPipeline.h"
#include "SkCmd.h"
#include "SkTex.h"
#include "SkModel.h"
// SkMesh mesh;
SkTex tex;
SkModel model;
void SkApp::Setup()
{
    // mesh.Init(this->base);
    // std::vector<D3D12_INPUT_ELEMENT_DESC> inputDescs;
    // mesh.SetupTriangle(inputDescs);
    model.Init(this->base);
    model.ImportModel(GetAssetFullPath("model/vk.obj"));
    tex.Init(GetAssetFullPath("texture/pic.jpg"));
    model.mesh.Setup();

    // tex.InitCheckerboard();
    pipeline.Setup(model.inputDescs);
    tex.Setup(base);
    cmd.AddMesh(&model.mesh);
    cmd.BuildCmdLists();
}
