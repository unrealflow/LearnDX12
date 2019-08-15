#include "SkApp.h"
#include "SkPipeline.h"
SkMesh mesh;
void SkApp::Setup()
{
    mesh.Init(this->base);
    mesh.SetupTriangle();
    pipeline.Setup(mesh.inputDescs);
    cmd.AddMesh(&mesh);
    cmd.BuildCmdLists();
}
