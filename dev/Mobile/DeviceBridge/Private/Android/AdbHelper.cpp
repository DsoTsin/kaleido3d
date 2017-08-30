#include "AdbHelper.h"
#include "DeviceMonitor.h"
#include <algorithm>

namespace k3d
{
    namespace mobi
    {
        namespace android
        {
            static os::IpAddress AdbHost("127.0.0.1:5037");
            char JdwpHandShake::HANDSHAKEBUFFER[14] = { 'J', 'D', 'W', 'P', '-', 'H', 'a', 'n', 'd', 's', 'h', 'a', 'k', 'e' };
            // profiler device port
            // @see StudioProfilerDeviceManager.java#createPerfdProxy
            int DEVICE_PERFD_PORT = 12389;

            int JdwpHandShake::FindHandShake(ByteBuffer & buf)
            {
                int count = buf.WrittenPosition();
                int i;
                if (count < 14)
                    return HANDSHAKE_NOTYET;
                for (i = 14 - 1; i >= 0; --i) {
                    if (buf.Get(i) != HANDSHAKEBUFFER[i])
                        return HANDSHAKE_BAD;
                }
                return HANDSHAKE_GOOD;
            }

            void JdwpHandShake::ConsumeHandShake(ByteBuffer & buf)
            {
                buf.Consume(14);
            }

            ShellProtocol::ShellProtocol(SocketChannel* s) : m_Sock(s)
            {
                buffer_[0] = kIdInvalid;
            }

            ShellProtocol::~ShellProtocol()
            {
            }

            bool ShellProtocol::Read()
            {
                // Only read a new header if we've finished the last packet.
                if (!bytes_left_)
                {
                    if (!m_Sock->Receive(buffer_, kHeaderSize)) {
                        return false;
                    }
                    length_t packet_length;
                    memcpy(&packet_length, &buffer_[1], sizeof(packet_length));
                    bytes_left_ = packet_length;
                    data_length_ = 0;
                }
                size_t read_length = std::min(bytes_left_, data_capacity());
                if (read_length && !m_Sock->Receive(data(), read_length))
                {
                    return false;
                }
                bytes_left_ -= read_length;
                data_length_ = read_length;
                return true;
            }

            bool ShellProtocol::Write(Id id, size_t length)
            {
                buffer_[0] = id;
                length_t typed_length = length;
                memcpy(&buffer_[1], &typed_length, sizeof(typed_length));
                return m_Sock->Send(buffer_, kHeaderSize + length);
            }

            SocketChannel::SocketChannel(const char* ipAddr) 
                : os::Socket(os::SockType::TCP)
                , m_SockAddr(ipAddr)
            {
                Create();
                if (ipAddr)
                {
                    m_Connected = Connect(m_SockAddr);
                }
            }

            SocketChannel::~SocketChannel()
            {
            }

            I32 SocketChannel::Write(String const & Data)
            {
                auto RetLen = Send(Data);
                m_Connected = RetLen >= 0;
                return RetLen;
            }

            I32 SocketChannel::Write(U8 * Data, int Size)
            {
                return Send((const char*)Data, Size);
            }

            I32 SocketChannel::Write(ByteBuffer const & buffer, int length)
            {
                auto RetLen = Send((const char*)buffer.m_Data, length);
                m_Connected = RetLen >= 0;
                return RetLen;
            }

            I32 SocketChannel::Read(void*Data, size_t Len)
            {
                auto RetLen = Receive(Data, Len);
                m_Connected = RetLen >= 0;
                return RetLen;
            }

            I32 SocketChannel::Read(ByteBuffer & buffer)
            {
                I32 RecvLen = Read(buffer.CurrentData(), buffer.Capacity());
                if (RecvLen > 0)
                {
                    buffer.m_WrittenPosition += RecvLen;
                }
                return RecvLen;
            }

            I32 SocketChannel::ReadInt()
            {
                I32 Value = 0;
                Read(&Value, 4);
                return Value;
            }

            String SocketChannel::ReadString(int strLen)
            {
                String str(strLen + 1, true);
                Read(str.Data(), strLen);
                return str;
            }

            String AdbHelper::FormAdbRequest(String const & Request)
            {
                String data;
                data.AppendSprintf("%04x%s", Request.Length(), *Request);
                return data;
            }

            SocketChannelPtr AdbHelper::Open(String const & DeviceSerialId, int DevicePort)
            {
                auto Socket = MakeShared<SocketChannel>(ADB_HOST);
                SetDevice(*Socket, DeviceSerialId);
                String Request = CreateForwardRequest("", DevicePort);
                Socket->Send(Request);
                AdbResponse Resp = ReadResponse(*Socket, false);
                if (!Resp.Succeed)
                {
                    Socket->Close();
                    return nullptr;
                }
                return Socket;
            }

            SocketChannelPtr AdbHelper::OpenServer(int InPort)
            {
                String Addr = "127.0.0.1";
                Addr.AppendSprintf(":%d", InPort);
                auto Socket = MakeShared<SocketChannel>(nullptr);
                Socket->SetBlocking(false);
                Socket->SetReuseAddr(true);
                Socket->Bind(os::IpAddress(Addr));
                Socket->Listen(100);
                int Erro = Socket->GetError();
                return Socket;
            }

            void AdbHelper::RunLogService(String const & DeviceSerialId, String const & LogName, LogCharReceive Callback, void* UserData)
            {
                if (!Callback)
                    return;
                SocketChannel Socket(ADB_HOST);
                if (!Socket.Connected())
                {
                    return;
                }
                SetDevice(Socket, DeviceSerialId);
                String StrReq = FormAdbRequest("shell:logcat");
                I32 Sent = Socket.Send(StrReq);
                AdbResponse Resp = ReadResponse(Socket, false);
                if (!Resp.Succeed)
                {
                    printf("error: %s in [%s].\n", *Resp.Message, __K3D_FUNC__);
                    return;
                }
                char* LogBuffer = (char*)GetDefaultAllocator().Alloc(16385);
                memset(LogBuffer, 0, 16385);
                while (true)
                {
                    I32 Count = Socket.Read(LogBuffer, 16384);
                    if (Count < 0 /*|| Logger->IsCancelled()*/)
                    {
                        break;
                    }
                    else if (Count == 0)
                    {
                        os::Sleep(5);
                    }
                    else
                    {
                        // Output chars to logger
                        for (int i = 0; i < Count; i++)
                        {
                            Callback(LogBuffer[i], UserData);
                        }
                    }
                }
                GetDefaultAllocator().DeAlloc(LogBuffer);
            }

            SocketChannelPtr AdbHelper::CreatePassThroughConnection(String const & DeviceSerialId, int PID)
            {
                auto Socket = MakeShared<SocketChannel>(ADB_HOST);
                SetDevice(*Socket, DeviceSerialId);
                String JdwpReq = CreateJdwpForwardRequest(PID);
                Socket->Send(JdwpReq);
                AdbResponse resp = ReadResponse(*Socket, false);
                if (!resp.Succeed)
                {
                    printf("unable to create forward jdwp request. \n");
                }
                return Socket;
            }

            void AdbHelper::ExecuteRemoteCommand(String const & DeviceSerialId, String const & Cmd, String& Output, int TimeOutInMS, int RecvRetryLimit)
            {
                SocketChannel Socket(ADB_HOST);
                if (!Socket.Connected())
                {
                    return;
                }
                SetDevice(Socket, DeviceSerialId);
                String Req = String::Format("shell:%s", *Cmd);
                String StrReq = FormAdbRequest(Req);
                I32 Sent = Socket.Send(StrReq);
                AdbResponse Resp = ReadResponse(Socket, false);
                if (!Resp.Succeed)
                {
                    printf("Execute remote command failed: %s in [%s].\n", *Resp.Message, __K3D_FUNC__);
                    return;
                }
                char* LogBuffer = (char*)GetDefaultAllocator().Alloc(16385);
                memset(LogBuffer, 0, 16385);
                int RetryReadTimes = 0;
                while (true)
                {
                    I32 Count = Socket.Read(LogBuffer, 16384);
                    if (Count < 0 || RetryReadTimes > RecvRetryLimit)
                    {
                        break;
                    }
                    else if (Count == 0)
                    {
                        os::Sleep(5);
                        RetryReadTimes++;
                    }
                    else
                    {
                        LogBuffer[Count] = '\0';
                        Output += LogBuffer;
                    }
                }
                GetDefaultAllocator().DeAlloc(LogBuffer);
            }

            void AdbHelper::SendSPSS(int bufferSize, int samplingInterval, int samplingIntervalTimeUnit)
            {
            }

            void AdbHelper::SendMPRQ()
            {
            }

            void AdbHelper::SendHello(SocketChannel * sock)
            {
                //auto rawbuf = ChunkHandler::AllocateBuffer(4);
                //JdwpPacket packet(rawbuf);
                //auto& buf = ChunkHandler::GetChunkDataBuffer(rawbuf);
                //// srvProtoVersion
                //buf.Put(0, (I32)1);
                //ChunkHandler::FinishChunkPacket(packet, ID_HELO, 4);
                //sock->SendJdwpPacket(packet);
            }

            void AdbHelper::ListJDWPProcesses(IDevice* InDevice, std::function<void(int pid)> callback)
            {
                SocketChannel sock(ADB_HOST);
                if (sock.Connected())
                {
                    DynArray<int> pids;
                    SendDeviceMonitoringRequest(sock, InDevice->GetSerialId());
                    ProcessingIncomingJDWPData(sock, pids);
                    if (pids.Count() > 0)
                    {
                        for (int pid : pids)
                        {
                            callback(pid);
                        }
                    }
                }
                else
                {
                    printf("ListJDWPProcesses failed.\n");
                }
            }

            void AdbHelper::SendDeviceMonitoringRequest(SocketChannel& sock, String const & serial)
            {
                SetDevice(sock, serial);
                sock.Write(FormAdbRequest("track-jdwp"));
                auto Rsp = ReadResponse(sock, false);
                if (!Rsp.Succeed)
                {
                    printf("failed to send monitor request, %s.", *Rsp.Message);
                }
            }

            void AdbHelper::ProcessingIncomingJDWPData(SocketChannel & sock, DynArray<int>& pids)
            {
                U32 PayloadLen = 0;
                char buff[5] = { 0 };
                sock.Receive(buff, 4);
                PayloadLen = strtoul(buff, nullptr, 16);
                if (PayloadLen == 0)
                {
                    printf("didn;t find jdwp process.\n");
                    return;
                }
                String prealloc(PayloadLen + 1, true);
                int len = sock.Receive(prealloc.Data(), PayloadLen);
                auto pos = prealloc.FindFirstOf("\n");
                String right(Move(prealloc));
                while (pos != String::npos)
                {
                    auto left = right.SubStr(0, pos);
                    int pid = atoi(*left);
                    pids.Append(pid);
                    right = right.SubStr(pos + 1, right.Length() - pos - 1);
                    pos = right.FindFirstOf("\n");
                }
            }

            void AdbHelper::SetDevice(SocketChannel & Socket, String const & SerialId)
            {
                if (SerialId.Length() > 0)
                {
                    String Msg = "host:transport:" + SerialId;
                    String DeviceQuery = FormAdbRequest(Msg);
                    I32 Sent = Socket.Send(DeviceQuery);
                    AdbResponse resp = ReadResponse(Socket, false);
                    if (!resp.Succeed)
                    {
                        printf("failed to connect to device (%s), reason: %s.\n", *SerialId, *resp.Message);
                    }
                }
            }

            String AdbHelper::CreateForwardRequest(String const & Addr, int Port)
            {
                String reqStr = "tcp:";
                if (Addr.Length() == 0)
                    reqStr.AppendSprintf("%d", Port);
                else
                    reqStr.AppendSprintf("%d:%s", Port, *Addr);
                return FormAdbRequest(reqStr);
            }

            String AdbHelper::CreateJdwpForwardRequest(int PID)
            {
                String reqStr = String::Format("jdwp:%d", PID);
                return FormAdbRequest(reqStr);
            }

            AdbResponse AdbHelper::ReadResponse(SocketChannel & Socket, bool ReadDiagString)
            {
                AdbResponse Resp;
                char Buffer[5] = { 0 };
                Socket.Receive(Buffer, 4);
                if (!strcmp(Buffer, "OKAY"))
                {
                    Resp.Succeed = true;
                }
                else
                {
                    Resp.Succeed = false;
                    ReadDiagString = true;
                }

                while (ReadDiagString)
                {
                    U32 PayloadLen = 0;
                    Socket.Receive(&PayloadLen, 4);
                    Resp.Message.Resize(PayloadLen + 1);
                    Socket.Receive(Resp.Message.Data(), PayloadLen);
                    break;
                }
                return Resp;
            }
}
    }
}