#include "MobileDeviceBridge.h"
#include "Android/Bridge.h"
#include "iOS/Bridge.h"

#include <unordered_map>

namespace k3d
{
    using DBMap = std::unordered_map<String, mobi::IBridgeService*>;

    class MDBPrivate
    {
    public:
        DBMap DeviceBridges;

        bool QueryDevices(mobi::DeviceList& List);
    };

    MobileDeviceBridge::MobileDeviceBridge()
        : d(new MDBPrivate)
    {}
    
    MobileDeviceBridge::~MobileDeviceBridge()
    {
        if (d)
        {
            delete d;
            d = nullptr;
        }
    }

    bool MobileDeviceBridge::QueryDevices(mobi::DeviceList& List)
    {
        return d->QueryDevices(List);
    }

    void MobileDeviceBridge::Register(String const & Name, mobi::IBridgeService * Service)
    {
        d->DeviceBridges[Name] = Service;
        Service->Start();
    }

    void MobileDeviceBridge::LaunchDeviceLogService()
    {
    }

    MobileDeviceBridge& GetMobileDeviceBridge()
    {
        static MobileDeviceBridge mdb;
        return mdb;
    }
    bool MDBPrivate::QueryDevices(mobi::DeviceList & List)
    {
        for (auto& Pair : DeviceBridges)
        {
            Pair.second->QueryDevices(List);
        }
        return true;
    }
}