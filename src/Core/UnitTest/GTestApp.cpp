#include "CoreMinimal.h"

class MyApp : public k3d::App {
public:
    MyApp()
        : k3d::App("My App", 800, 600)
    {
    }

    void OnProcess(k3d::Message& message) override
    {
    }
};

#if _WIN32
int WinMain(
    void*,
    void*,
    char*,
    int)
#else
int main(int argc, const char* argv[])
#endif
{
    MyApp app;
    return k3d::RunApplication(app, "My App");
}