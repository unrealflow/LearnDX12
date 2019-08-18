#include "SkApp.h"
#include "SkPipeline.h"
#include "SkCmd.h"
#include "SkTex.h"
#include "SkModel.h"
SkMesh mesh;
SkTex tex;
void SkApp::Setup()
{
    mesh.Init(this->base);
    std::vector<D3D12_INPUT_ELEMENT_DESC> inputDescs;
    mesh.SetupTriangle(inputDescs);
    tex.Init(GetAssetFullPath("texture/pic.jpg"));
    // tex.InitCheckerboard();
    pipeline.Setup(inputDescs);
    tex.Setup(base);
    cmd.AddMesh(&mesh);
    cmd.BuildCmdLists();
}
