#include <MobileDeviceBridge.h>

#if _WIN32
#pragma comment(linker,"/subsystem:console")
#endif

using namespace k3d;
using namespace k3d::mobi;

const char* PlatformString(EPlatform Platform)
{
    switch (Platform)
    {
    case k3d::mobi::EPlatform::iOS:
        return "iOS";
    case k3d::mobi::EPlatform::Android:
        return "Android";
    default:
        return "Unknown";
    }
}

int main(int argc, const char* argv[])
{
    DeviceList List;
    GetMobileDeviceBridge()
        .QueryDevices(List);
    // Print All Devices
    for (auto& Device : List)
    {
        printf("Found device: %s, platform: %s\n",
            *Device->GetSerialId(),
            PlatformString(Device->GetPlatform()));
    }
    // Connect To All Devices
    return 0;
}