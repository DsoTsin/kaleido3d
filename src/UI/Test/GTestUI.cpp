#include "ngfx_shell.h"
#include "CoreMinimal.h"

class MyUITestApp : public k3d::App {
public:
    MyUITestApp()
        : k3d::App("Test UI", 800, 600)
    {
    }

    void OnProcess(k3d::Message& message) override
    {
    }
};

#if _WIN32
int WinMain(void*, void*, char*, int)
#else
int main(int argc, const char* argv[])
#endif
{
    MyUITestApp app;
    return k3d::RunApplication(app, "UI Test App");
}