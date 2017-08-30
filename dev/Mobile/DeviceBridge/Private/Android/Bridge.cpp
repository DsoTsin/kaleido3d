#include "Bridge.h"
#include "DeviceMonitor.h"
#include <grpc/grpc.h>
#include <grpc++/channel.h>
#include <grpc++/client_context.h>
#include <grpc++/create_channel.h>
#include <grpc++/security/credentials.h>

#include "gpu.grpc.pb.h"

//#include <map>
#include <unordered_map>

#include <WinSock2.h>

namespace k3d
{
    namespace mobi
    {
        namespace android
        {
            class SyncConnection : public SocketChannel
            {
            public:
                SyncConnection(String const& serial) : SocketChannel()
                {
                    if (Connected())
                    {
                        AdbHelper::SetDevice(*this, serial);
                        auto Srv = AdbHelper::FormAdbRequest("sync:");
                        Send(Srv);
                        AdbResponse rsp = AdbHelper::ReadResponse(*this, false);
                        if (!rsp.Succeed)
                        {
                            printf("sync error: %s.\n", *rsp.Message);
                        }
                    }
                }

                ~SyncConnection()
                {}

                bool SendReq(int id, const char* pathMode)
                {

                }
            private:
            };

            static AndroidBridge ADB;

            AndroidBridge::AndroidBridge()
            {
                GetMobileDeviceBridge().Register("Android", this);
            }

            AndroidBridge::~AndroidBridge()
            {
            }

            void AndroidBridge::Start()
            {
                m_BridgeThread = MakeShared<os::Thread>([this]()
                {
                    while (!m_Quit.Get())
                    {
                        ListDevices();
                        os::Sleep(3000);
                    }
                }, "AndroidBridge");
            }

            void AndroidBridge::Stop()
            {
                m_Quit.Set(true);
                m_BridgeThread->Join();
            }

            bool AndroidBridge::QueryDevices(mobi::DeviceList & devices)
            {
                os::Mutex::AutoLock lock(&m_DeviceLock);
                devices.AddAll(m_Devices);
                return true;
            }

            void AndroidBridge::ListDevices()
            {
                mobi::DeviceList devices;
                SocketChannel sock(ADB_HOST);
                if (!sock.Connected())
                {
                    printf("adb server not existed.\n");
                    return;
                }
                sock.Write(AdbHelper::FormAdbRequest("host:track-devices"));
                AdbResponse resp = AdbHelper::ReadResponse(sock, false /* readDiagString */);
                if (resp.Succeed == false)
                {
                    printf("adb refused, %s. \n", *resp.Message);
                    return;
                }
                else
                {
                    I32 RespLen = 0;
                    ReadLength(sock, RespLen);
                    ProcessIncomingDeviceData(sock, RespLen, devices);
                }
                if (devices.Count() > 0)
                {
                    for (IDevice* device : devices)
                    {
                        auto rDev = static_cast<Device*>(device);
                        String output;
                        rDev->ExecuteCommand("getprop", output);
                        const char* content = *output;
                        bool capture = false;
                        bool capture_end = false;
                        int pos_capture_start = 0;
                        int char_count = 0;
                        bool isKey = true;
                        std::map<std::string, String> propDict;
                        std::string lastKey;
                        while (*content != '\0')
                        {
                            if (*content == '[')
                            {
                                capture = true;
                                capture_end = false;
                                pos_capture_start = char_count;
                            }
                            else if (*content == ']')
                            {
                                capture_end = true;
                                capture = false;
                                std::string str(*output + pos_capture_start + 1, char_count - pos_capture_start - 1);
                                if (isKey)
                                {
                                    lastKey = str;
                                    propDict[str];
                                }
                                else
                                {
                                    propDict[lastKey] = String(str.c_str());
                                }
                                isKey = !isKey;
                            }
                            else if (*content == ':')
                            {

                            }
                            else if (*content == '\n')
                            {
                                // new line
                            }
                            content++;
                            char_count++;
                        }

                        auto ver = propDict["ro.build.version.release"];
                        auto sdk = propDict["ro.build.version.sdk"];
                        auto model = propDict["ro.product.model"];
                        rDev->m_Desc = model + " " + ver + " " + sdk;
                        // list apps
                        rDev->ListDebuggableApps();
                    }
                    os::Mutex::AutoLock lock(&m_DeviceLock);
                    m_Devices.Swap(devices);
                }
            }

            void AndroidBridge::ReadLength(SocketChannel& Channel, I32 & Length)
            {
                char Buffer[4] = { 0 };
                Channel.Read(Buffer, 4);
                sscanf(Buffer, "%04x", &Length);
            }

            void AndroidBridge::ProcessIncomingDeviceData(SocketChannel& Channel, I32 Length, mobi::DeviceList & devices)
            {
                if (Length > 0)
                {
                    String Data(Length + 1, true);
                    Channel.Read(Data.Data(), Length);
                    auto Pos = Data.FindFirstOf("\n");
                    if (Pos != String::npos)
                    {
                        String RemainStr(Move(Data));
                        while (Pos != String::npos)
                        {
                            String OneDevLine = RemainStr.SubStr(0, Pos);
                            auto Sep = OneDevLine.FindFirstOf("\t");
                            String SerialId = OneDevLine.SubStr(0, Sep);
                            devices.Append(new android::Device(this, SerialId));
                            if (Pos != Length - 1)
                            {
                                RemainStr = RemainStr.SubStr(Pos + 1, RemainStr.Length() - Pos - 1);
                                Pos = RemainStr.FindFirstOf("\n");
                            }
                            else
                            {
                                break;
                            }
                        }
                    }
                }
            }

            class CPUProfiler : public ICPUProfiler
            {
            public:
                CPUProfiler();
                void        StartMonitor(ProcessInfo const& Process) override;
                void        StopMonitor(ProcessInfo const& Process) override;
                void        StartProfile(ProcessInfo const& Process) override;
                void        StopProfile(ProcessInfo const& Process) override;
                CPUDatas    GetData() override;
            };

            class GPUProfiler : public IGPUProfiler
            {
            public:
                GPUProfiler(std::shared_ptr<grpc::Channel>);
                void        StartMonitor(ProcessInfo const& Process) override;
                void        StopMonitor(ProcessInfo const& Process) override;
                void        StartProfile(ProcessInfo const& Process) override;
                void        StopProfile(ProcessInfo const& Process) override;
                GPUDatas    GetData() override;

            private:
                std::shared_ptr<profiler::proto::GpuService::Stub> m_Stub;
            };

            using ClientList = DynArray<SharedPtr<Client>>;

            struct DeviceImpl
            {
                DeviceImpl(const char* InSerial) 
                    : Serial(InSerial)
                    , Logger(nullptr)
                    , Log(this, &DeviceImpl::OnNewLogLine)
                {
                    bIsOffline = false;
                }

                void StartLog()
                {
                    LoggerThread = MakeShared<os::Thread>([=]() {
                        AdbHelper::RunLogService(Serial, "events", LogCallback, this);
                    }, "AndroidLog");
                    LoggerThread->Start();
                }

                static void LogCallback(char C, void *UserData)
                {
                    DeviceImpl* Device = reinterpret_cast<DeviceImpl*>(UserData);
                    Device->ReceiveLog(C);
                }

                void ReceiveLog(char C)
                {
                    Log << C;
                }

                void OnNewLogLine(String && LogLine)
                {
                    // Parse log line
                    if (Logger)
                    {
                        auto Pos = LogLine.FindFirstOf("DIWE");
                        if (Pos != String::npos)
                        {
                            ELogLevel Level = ELogLevel::Info;
                            switch (LogLine[Pos])
                            {
                            case 'I':
                                break;
                            case 'D':
                                Level = ELogLevel::Debug;
                                break;
                            case 'W':
                                Level = ELogLevel::Warn;
                                break;
                            case 'E':
                                Level = ELogLevel::Error;
                                break;
                            default:
                                break;
                            }
                            Logger->Log(Level, ".", *LogLine);
                        }
                    }
                }

                void CreateProfilerChannel()
                {
                    if (!Channel)
                    {
                        Channel = grpc::CreateChannel("tcp:12389",
                            grpc::InsecureChannelCredentials());
                        //adb.exe forward tcp:12389 localabstract:AndroidStudioProfiler
                    }
                }

                void Disconnect()
                {
                    bIsOffline.Set(true);
                    MonitorThread->Join();
                }

                void AddClient(JdwpAgentPtr Agent);
                void NotifyClientsChanged();

                const char*             Serial;
                AtomicBool              bIsOffline = true;
                mutable ILogger*        Logger;
                SharedPtr<os::Thread>   LoggerThread;
                LogBuffer<DeviceImpl>   Log;
                std::shared_ptr<grpc::Channel>Channel;
                JdwpAgents              Clients;
                SharedPtr<Debugger>     Debuggers;
                os::SocketSet           ReadSet;
                ThreadPtr               MonitorThread;
            };

            Device::Device(AndroidBridge* InBridge, String const& InSerialId)
                : m_Bridge(InBridge)
                , m_SerialId(InSerialId)
            {
                d = new DeviceImpl(*m_SerialId);
            }
            Device::~Device()
            {
                if (d)
                {
                    delete d;
                    d = nullptr;
                }
            }
            bool Device::IsConnected() const
            {
                return false;
            }
            void Device::Connect()
            {
                d->bIsOffline.Set(false);
            }
            void Device::Disconnect()
            {
                d->Disconnect();
            }
            void Device::StartLogService()
            {
                d->StartLog();
            }
            EPlatform Device::GetPlatform() const
            {
                return EPlatform::Android;
            }
            String Device::GetSerialId() const
            {
                return m_SerialId;
            }
            String Device::GetDesc() const
            {
                return m_Desc;
            }

            void Device::InstallLogger(ILogger * InLogger)
            {
                __intrinsics__::AtomicCASPointer((void**)&d->Logger, InLogger, d->Logger);
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
                Processes procs;
                DynArray<int> pids;
                
                AdbHelper::ListJDWPProcesses(this, [&pids, this](int pid) 
                {
                    auto Client = MakeShared<JdwpClient>(this, pid);
                    if (Client->SendHandShake())
                    {
                        d->AddClient(Client);
                        pids.Append(pid);
                    }
                    else
                    {
                        printf("failed to send handshake to %d.", pid);
                    }
                });
                return procs;
            }
            ProcessorInfo Device::GetProcessorInfo()
            {
                return ProcessorInfo();
            }
            IProfiler * Device::CreateProfiler(EProfileCategory const &)
            {
                return nullptr;
            }

            void Device::AddClient(JdwpAgentPtr client)
            {
                d->Clients.Append(client);
            }

            void Device::ExecuteCommand(String const & cmd, String&out)
            {
                AdbHelper::ExecuteRemoteCommand(m_SerialId, cmd, out, 500);
            }

            void Device::ListDebuggableApps()
            {
                DynArray<int> pids;
                AdbHelper::ListJDWPProcesses(this, [&pids, this](int pid)
                {
                    auto Client = MakeShared<JdwpClient>(this, pid);
                    if (Client->SendHandShake())
                    {
                        d->AddClient(Client);
                        pids.Append(pid);
                    }
                    else
                    {
                        printf("failed to send handshake to %d.", pid);
                    }
                });
            }

            using namespace profiler::proto;

            CPUProfiler::CPUProfiler()
            {}
            void CPUProfiler::StartMonitor(ProcessInfo const & Process)
            {
            }
            void CPUProfiler::StopMonitor(ProcessInfo const & Process)
            {
            }
            void CPUProfiler::StartProfile(ProcessInfo const & Process)
            {
            }
            void CPUProfiler::StopProfile(ProcessInfo const & Process)
            {
            }
            CPUDatas CPUProfiler::GetData()
            {
                return CPUDatas();
            }
            GPUProfiler::GPUProfiler(std::shared_ptr<grpc::Channel> Channel)
                : m_Stub(profiler::proto::GpuService::NewStub(Channel))
            {
            }
            void GPUProfiler::StartMonitor(ProcessInfo const & Process)
            {
                grpc::ClientContext context;
                //m_Stub->StartMonitoringApp(&context,)
            }
            void GPUProfiler::StopMonitor(ProcessInfo const & Process)
            {
                grpc::ClientContext context;
            }
            void GPUProfiler::StartProfile(ProcessInfo const & Process)
            {
                grpc::ClientContext context;
            }
            void GPUProfiler::StopProfile(ProcessInfo const & Process)
            {
                grpc::ClientContext context;
            }
            GPUDatas GPUProfiler::GetData()
            {
                grpc::ClientContext context;
                GpuDataRequest request;
                GpuDataResponse response;
                m_Stub->GetData(&context, request, &response);
                return GPUDatas();
            }
            void DeviceImpl::AddClient(JdwpAgentPtr Agent)
            {
                Clients.Append(Agent);
                //
                if (Agent->GetType() == JdwpAgentType::Client)
                {
                    auto c = StaticPointerCast<JdwpClient>(Agent);
                    auto d = c->GetDebugger();
                    Clients.Append(d);
                    d->Register(ReadSet);
                }

                ReadSet.Add(Agent->GetSock().Get(), Agent.Get());
                Agent.ForceAddRef();
                //
                NotifyClientsChanged();
            }
            void DeviceImpl::NotifyClientsChanged()
            {
                if (!MonitorThread)
                {
                    MonitorThread = MakeShared<os::Thread>([this]() {
                        while (!bIsOffline)
                        {
                            int nRet = os::SocketSet::Select(ReadSet, 10000);
                            if (nRet < 0)
                            {
                            }
                            if (nRet > 0)
                            {
                                for (auto Iter = ReadSet.begin(); Iter; ++Iter)
                                {
                                    void* Ptr = Iter.UserData();
                                    if (Ptr)
                                    {
                                        IJdwpAgent* Agent = (IJdwpAgent*)Ptr;
                                        switch (Agent->GetType())
                                        {
                                        case JdwpAgentType::Debugger:
                                        {
                                            Debugger * d = static_cast<Debugger*>(Agent);
                                            d->Accept(ReadSet);
                                            break;
                                        }
                                        case JdwpAgentType::Client:
                                            break;
                                        }
                                    }
                                }
                                for (auto Client : Clients)
                                {
                                    switch (Client->GetType())
                                    {
                                    case JdwpAgentType::Client:
                                    {
                                        auto c = StaticPointerCast<JdwpClient>(Client);
                                        c->ReadIn();
                                        auto pkt = c->GetPacket();
                                        while (pkt)
                                        {
                                            if (pkt->IsDDMPkt())
                                            {

                                            }
                                            else if (pkt->IsReply() && pkt->GetId())
                                            {
                                                auto Handler = c->GetAndConsume(pkt->GetId());
                                                if (pkt->IsEmpty())
                                                {
                                                    printf("Packet is Empty!!\n");
                                                    pkt->Dump();
                                                }
                                                else if (pkt->IsError())
                                                {
                                                    printf("Packet has Error!!\n");
                                                }
                                                else
                                                {
                                                    Handler->HandleChunk(c, pkt);
                                                }
                                            }
                                            else
                                            {
                                                printf("Unknown Packet!!\n");
                                            }
                                            pkt = c->GetPacket();
                                        }
                                        break;
                                    }
                                    case JdwpAgentType::Debugger:
                                    {
                                        auto d = StaticPointerCast<Debugger>(Client);
                                        d->Accept(ReadSet);
                                        break;
                                    }
                                    }
                                }
                            }

                        }
                    }, "Android DM");
                }
            }
}
    }
}