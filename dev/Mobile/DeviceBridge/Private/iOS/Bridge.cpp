#include "Bridge.h"
#include <libimobiledevice/libimobiledevice.h>
#include <libimobiledevice/lockdown.h>

#include <libimobiledevice/installation_proxy.h>
#include <libimobiledevice/notification_proxy.h>
#include <libimobiledevice/afc.h>
#include <libimobiledevice/syslog_relay.h>

#include <plist/plist.h>
#include "..\Android\Bridge.h"

#define ITUNES_METADATA_PLIST_FILENAME "iTunesMetadata.plist"


namespace k3d
{
    namespace mobi
    {
        static AMService AMS;

        AMService::AMService()
        {
            GetMobileDeviceBridge().Register("iOS", this);
        }

        AMService::~AMService()
        {
        }

        void AMService::Start()
        {
        }

        bool AMService::QueryDevices(mobi::DeviceList & devices)
        {
            char **DeviceUUIDs = NULL;
            int NumDevices = 0;
            idevice_get_device_list(&DeviceUUIDs, &NumDevices);
            if (NumDevices > 0)
            {
                for (int i = 0; i < NumDevices; i++)
                {
                    char* UUID = DeviceUUIDs[i];
                    devices.Append(new ios::Device(UUID, this));
                }
                idevice_device_list_free(DeviceUUIDs);
            }
            return false;
        }

        namespace ios
        {

            struct DeviceImpl
            {
                DeviceImpl(const char* InUUID)
                    : UUID(InUUID)
                    , Logger(nullptr)
                    , Buffer(this, &DeviceImpl::OnLogLine)
                {
                    idevice_new(&Device, UUID);
                }
                ~DeviceImpl()
                {
                    idevice_free(Device);
                }

                void StartLog()
                {
                    LoggerThread = MakeShared<os::Thread>([=]() {
                        LogStop = false;
                        idevice_event_subscribe(EventCallback, this);
                        while (!LogStop) {
                            os::Sleep(1);
                        }
                        idevice_event_unsubscribe();
                        StopLoggingInternal();
                    }, "iOSLogger");
                    LoggerThread->Start();
                }

                int StartLoggingInternal()
                {
                    syslog_relay_error_t serr = SYSLOG_RELAY_E_UNKNOWN_ERROR;
                    serr = syslog_relay_client_start_service(Device, &SysLog, "idevicesyslog");
                    if (serr != SYSLOG_RELAY_E_SUCCESS) {
                        fprintf(stderr, "ERROR: Could not start service com.apple.syslog_relay.\n");
                        return -1;
                    }

                    serr = syslog_relay_start_capture(SysLog, LogCallBack, this);
                    if (serr != SYSLOG_RELAY_E_SUCCESS) {
                        fprintf(stderr, "ERROR: Unable tot start capturing syslog.\n");
                        syslog_relay_client_free(SysLog);
                        SysLog = NULL;
                        return -1;
                    }

                    fprintf(stdout, "[connected]\n");
                    fflush(stdout);

                    return 0;
                }

                void StopLoggingInternal()
                {
                    if (SysLog) {
                        syslog_relay_client_free(SysLog);
                        SysLog = NULL;
                    }
                }

                void OnLogLine(String&& NewLine)
                {
                    // Parse log line
                    if (Logger)
                    {
                        Logger->Log(ELogLevel::Info, "dd", *NewLine);
                    }
                }

                static void LogCallBack(char c, void *data)
                {
                    DeviceImpl* Device = reinterpret_cast<DeviceImpl*>(data);
                    Device->ReceiveLogChar(c);
                }

                void ReceiveLogChar(char c)
                {
                    Buffer << c;
                }

                void SetLogger(ILogger* InLogger)
                {
                    Logger = InLogger;
                }

                static void EventCallback(const idevice_event_t* event, void* data)
                {
                    DeviceImpl* Device = reinterpret_cast<DeviceImpl*>(data);
                    if (event->event == IDEVICE_DEVICE_ADD) {
                        if (!Device->SysLog) {
                            if (strcmp(Device->UUID, event->udid) == 0) 
                            {
                                if (Device->StartLoggingInternal() != 0) {
                                    fprintf(stderr, "Could not start logger for udid %s\n", Device->UUID);
                                }
                            }
                        }
                    }
                    else if (event->event == IDEVICE_DEVICE_REMOVE) {
                        if (Device->SysLog && (strcmp(Device->UUID, event->udid) == 0)) {
                            Device->StopLoggingInternal();
                            fprintf(stdout, "[disconnected]\n");
                        }
                    }
                }

                const char*             UUID = NULL;
                idevice_t               Device = NULL;
                syslog_relay_client_t   SysLog = NULL;

                SharedPtr<os::Thread>   LoggerThread;
                bool                    LogStop;
                mutable ILogger*        Logger;
                LogBuffer<DeviceImpl>   Buffer;
            };

            Device::Device(String const& InUuid, AMService* InAMS)
                : m_Bridge(InAMS)
                , m_UUId(InUuid)
            {
                d = new DeviceImpl(*m_UUId);
            }
            Device::~Device()
            {
                if (d)
                {
                    delete d;
                }
            }
            bool Device::IsConnected() const
            {
                return false;
            }
            void Device::Connect()
            {
            }
            void Device::StartLogService()
            {
                d->StartLog();
            }
            EPlatform Device::GetPlatform() const
            {
                return EPlatform::iOS;
            }
            String Device::GetSerialId() const
            {
                return m_UUId;
            }

            String Device::GetDesc() const
            {
                return "";
            }
            void Device::InstallLogger(ILogger * InLogger)
            {
                d->Logger = InLogger;
            }
            bool Device::InstallApp(String const & AppHostPath)
            {
                return false;
            }
            bool Device::Upload(String const & HostPath, String const & TargetPath)
            {
                return false;
            }
            bool Device::Download(String const & TargetPath, String const & HostPath)
            {
                return false;
            }
            Processes Device::ListAllRunningProcess()
            {
                return Processes();
            }
            ProcessorInfo Device::GetProcessorInfo()
            {
                return ProcessorInfo();
            }
            IProfiler * Device::CreateProfiler(EProfileCategory const &)
            {
                return nullptr;
            }
        }
}
}
