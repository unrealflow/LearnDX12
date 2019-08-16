#include "SkApp.h"


class SkRender : public SkApp
{
public:
    SkRender() : SkApp(960, 540, "LearnD3D12")
    {
    }
};

#ifdef WIN_MAIN
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
#else
int main()
#endif
{
   SkRender skApp;
    try
    {
        skApp.Run();
    }
    catch (const std::exception &e)
    {
        std::cerr <<"Run Error! "<<e.what() << '\n';
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
