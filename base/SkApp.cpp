#include "SkApp.h"
#include "SkPipeline.h"
#include "SkCmd.h"
#include "SkTex.h"
SkMesh mesh;
SkTex tex;
void SkApp::Setup()
{
    mesh.Init(this->base);
    mesh.SetupTriangle();
    tex.Init(GetAssetFullPath("texture/pic.jpg"));
    // tex.InitCheckerboard();
    pipeline.Setup(mesh.inputDescs);
    tex.Setup(base);
    cmd.AddMesh(&mesh);
    cmd.BuildCmdLists();
}
