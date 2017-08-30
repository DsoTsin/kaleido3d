#pragma once
#include <Core/CoreMinimal.h>

#ifndef MOBILEDEVICEBRIDGE_API
#define MOBILEDEVICEBRIDGE_API __declspec(dllexport)
#endif

namespace k3d
{
    namespace mobi
    {
        enum class EPlatform
        {
            iOS, Android
        };

        enum class EProfileCategory
        {
            CPU_Usage,
            GPU_Usage,
            Memory_Usage,
            Memory_BandWidth,
        };

        struct MOBILEDEVICEBRIDGE_API ProcessInfo
        {
            String  Name;
            U32     Id;
        };
        using Processes = DynArray<ProcessInfo>;

        struct MOBILEDEVICEBRIDGE_API CPUData
        {
            int Usage;
        };
        using CPUDatas = DynArray<CPUData>;

        struct MOBILEDEVICEBRIDGE_API GPUData
        {
            int Usage;
        };
        using GPUDatas = DynArray<CPUData>;

        struct MOBILEDEVICEBRIDGE_API ProcessorInfo
        {
            int     NumCores;
            String  Vendor;
            String  GPUInfo;
            String  GrfxAPIInfo;
        };

        class MOBILEDEVICEBRIDGE_API IProfiler
        {
        public:
            virtual ~IProfiler() {}
            virtual void StartMonitor(ProcessInfo const& Process) {}
            virtual void StopMonitor(ProcessInfo const& Process) {}
            virtual void StartProfile(ProcessInfo const& Process) {}
            virtual void StopProfile(ProcessInfo const& Process) {}
        };

        class MOBILEDEVICEBRIDGE_API ICPUProfiler : public IProfiler
        {
        public:
            virtual ~ICPUProfiler() {}
            virtual CPUDatas GetData() = 0;
        };

        class MOBILEDEVICEBRIDGE_API IGPUProfiler : public IProfiler
        {
        public:
            virtual ~IGPUProfiler() {}
            virtual GPUDatas GetData() = 0;
        };

        class MOBILEDEVICEBRIDGE_API IDevice
        {
        public:
            virtual                 ~IDevice() {}
            virtual bool            IsConnected() const = 0;
            virtual void            Connect()           = 0;
            virtual void            Disconnect() {}
            virtual EPlatform       GetPlatform() const = 0;
            virtual String          GetSerialId() const = 0;
            virtual String          GetDesc() const = 0;

            virtual void            InstallLogger(ILogger* InLogger) = 0;
            virtual void            StartLogService() = 0;
            virtual bool            InstallApp(String const& AppHostPath) = 0;
            virtual bool            Upload(String const& HostPath, String const& TargetPath) = 0;
            virtual bool            Download(String const& TargetPath, String const& HostPath) = 0;

            virtual Processes       ListAllRunningProcess() = 0;

            virtual ProcessorInfo   GetProcessorInfo() = 0;
            virtual IProfiler*      CreateProfiler(EProfileCategory const&) { return nullptr; }
        };

        typedef DynArray<IDevice*> DeviceList;

        class MOBILEDEVICEBRIDGE_API IBridgeService
        {
        public:
            virtual ~IBridgeService() {}
            virtual void Start() {}
            virtual void Stop() {}
            virtual bool QueryDevices(mobi::DeviceList& devices) = 0;
        };
    }

    class MOBILEDEVICEBRIDGE_API MobileDeviceBridge
    {
    public:
        MobileDeviceBridge();
        ~MobileDeviceBridge();
        /**
         * List all mobile devices connected to host
         */
        bool QueryDevices(mobi::DeviceList& devices);
        /**
         * Register mobile bridge service 
         */
        void Register(String const& Name, mobi::IBridgeService* Service);

        /**/
        void LaunchDeviceLogService();

    private:
        class MDBPrivate* d;
    };

    extern MOBILEDEVICEBRIDGE_API MobileDeviceBridge& GetMobileDeviceBridge();
}
