#pragma once
#include "MobileDeviceBridge.h"
#include "Common/LogBuffer.h"

namespace k3d
{
    namespace mobi
    {
        // Apple Mobile Device Service provided by iTunes
        class AMService : public IBridgeService
        {
        public:
            AMService();
            ~AMService();

            void Start();

            bool QueryDevices(mobi::DeviceList& devices) override;

        private:

        };
        namespace ios
        {
            struct DeviceImpl;

            class Device : public IDevice
            {
            public:
                Device(String const& InUuid, AMService* InAMS);
                ~Device() override;

                bool            IsConnected() const override;
                void            Connect() override;
                void            StartLogService() override;
                EPlatform       GetPlatform() const override;
                String          GetSerialId() const override;
                String          GetDesc() const override;
                void            InstallLogger(ILogger* InLogger) override;
                bool            InstallApp(String const& AppHostPath) override;
                bool            Upload(String const& HostPath, String const& TargetPath) override;
                bool            Download(String const& TargetPath, String const& HostPath) override;

                Processes       ListAllRunningProcess() override;
                ProcessorInfo   GetProcessorInfo() override;
                IProfiler*      CreateProfiler(EProfileCategory const&) override;
            private:
                String          m_UUId;
                AMService*      m_Bridge;
                // PIML
                DeviceImpl*     d;
            };
        }
    }
}