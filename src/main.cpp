#include "SkApp.h"


class SkRender : public SkApp
{
public:
    SkRender() : SkApp(960, 540, "LearnD3D12")
    {
    }
};

int main()
{
    SkRender skApp;
    try
    {
        skApp.Run();
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
