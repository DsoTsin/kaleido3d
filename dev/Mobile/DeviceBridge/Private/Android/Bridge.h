#pragma once
#include "MobileDeviceBridge.h"
#include "Common/LogBuffer.h"
#include "AdbHelper.h"

namespace k3d
{
    namespace mobi
    {
        namespace android
        {
            class AndroidBridge : public IBridgeService
            {
            public:
                AndroidBridge();
                ~AndroidBridge();

                void        Start() override;
                void        Stop() override;
                bool        QueryDevices(mobi::DeviceList& devices) override;

            private:
                os::Mutex           m_DeviceLock;
                mobi::DeviceList    m_Devices;
                AtomicBool          m_Quit;

                void ListDevices();

                SharedPtr<os::Thread> m_BridgeThread;

                static void ReadLength(SocketChannel& Channel, I32& Length);
                void        ProcessIncomingDeviceData(SocketChannel& Channel, I32 Length, mobi::DeviceList& devices);
            };

            struct DeviceImpl;
            using JdwpAgents = DynArray<JdwpAgentPtr>;
            class Client;
            using ClientPtr = SharedPtr<Client>;
            using ClientList = DynArray<ClientPtr>;
            using ThreadPtr = SharedPtr<os::Thread>;

            class Device : public IDevice
            {
            public:
                explicit Device(AndroidBridge* InBridge, String const& InSerialId);
                ~Device() override;
                
                bool            IsConnected() const override;
                void            Connect() override;
                void            Disconnect() override;
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

                void            AddClient(JdwpAgentPtr client);
                void            ExecuteCommand(String const& cmd, String&out);

                friend class    AndroidBridge;
                friend class    JdwpClient;
            private:
                void            ListDebuggableApps();

                String          m_SerialId;
                String          m_Desc;
                AndroidBridge*  m_Bridge;
                DeviceImpl*     d;
            };

        }
    }
}