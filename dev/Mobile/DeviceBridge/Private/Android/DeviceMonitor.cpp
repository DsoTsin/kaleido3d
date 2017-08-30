#include "DeviceMonitor.h"
#include "Bridge.h"

#if _WIN32
#include <WinSock2.h>
#endif

namespace k3d
{
    namespace mobi
    {
        namespace android
        {
            HelloChunk::HelloChunk() : ChunkHandler(ID_HELO,4)
            {
            }

            void HelloChunk::HandleChunk(JdwpAgentPtr agent, JdwpPacketPtr packet)
            {
                const U8* ChunkData = GetChunkData(*packet);
                int Version = Read4BE(&ChunkData);
                int PID = Read4BE(&ChunkData);
                int VmIdentLen = Read4BE(&ChunkData);
                int AppNameLen = Read4BE(&ChunkData);
                String Vm = ReadBEString(&ChunkData, VmIdentLen);
                String AppName = ReadBEString(&ChunkData, AppNameLen);
                
                auto Client = StaticPointerCast<JdwpClient>(agent);
                Client->SetAppName(AppName);
                // Newer devices send user id in the APNM packet.
                //int userId = -1;
                //boolean validUserId = false;
                //if (data.hasRemaining()) {
                //    try {
                //        userId = data.getInt();
                //        validUserId = true;
                //    }
                //    catch (BufferUnderflowException e) {
                //        // five integers + two utf-16 strings
                //        int expectedPacketLength = 20 + appNameLen * 2 + vmIdentLen * 2;

                //        Log.e("ddm-hello", "Insufficient data in HELO chunk to retrieve user id.");
                //        Log.e("ddm-hello", "Actual chunk length: " + data.capacity());
                //        Log.e("ddm-hello", "Expected chunk length: " + expectedPacketLength);
                //    }
                //}

                //// check if the VM has reported information about the ABI
                //boolean validAbi = false;
                //String abi = null;
                //if (data.hasRemaining()) {
                //    try {
                //        int abiLength = data.getInt();
                //        abi = ByteBufferUtil.getString(data, abiLength);
                //        validAbi = true;
                //    }
                //    catch (BufferUnderflowException e) {
                //        Log.e("ddm-hello", "Insufficient data in HELO chunk to retrieve ABI.");
                //    }
                //}

                //boolean hasJvmFlags = false;
                //String jvmFlags = null;
                //if (data.hasRemaining()) {
                //    try {
                //        int jvmFlagsLength = data.getInt();
                //        jvmFlags = ByteBufferUtil.getString(data, jvmFlagsLength);
                //        hasJvmFlags = true;
                //    }
                //    catch (BufferUnderflowException e) {
                //        Log.e("ddm-hello", "Insufficient data in HELO chunk to retrieve JVM flags");
                //    }
                //}

                //boolean nativeDebuggable = false;
                //if (data.hasRemaining()) {
                //    try {
                //        byte nativeDebuggableByte = data.get();
                //        nativeDebuggable = nativeDebuggableByte == 1;
                //    }
                //    catch (BufferUnderflowException e) {
                //        Log.e("ddm-hello", "Insufficient data in HELO chunk to retrieve nativeDebuggable");
                //    }
                //}

                //Log.d("ddm-hello", "HELO: v=" + version + ", pid=" + pid
                //    + ", vm='" + vmIdent + "', app='" + appName + "'");

                //ClientData cd = client.getClientData();

                //if (cd.getPid() == pid) {
                //    cd.setVmIdentifier(vmIdent);
                //    cd.setClientDescription(appName);
                //    cd.isDdmAware(true);

                //    if (validUserId) {
                //        cd.setUserId(userId);
                //    }

                //    if (validAbi) {
                //        cd.setAbi(abi);
                //    }

                //    if (hasJvmFlags) {
                //        cd.setJvmFlags(jvmFlags);
                //    }

                //    cd.setNativeDebuggable(nativeDebuggable);
                //}
                //else {
                //    Log.e("ddm-hello", "Received pid (" + pid + ") does not match client pid ("
                //        + cd.getPid() + ")");
                //}

                //client = checkDebuggerPortForAppName(client, appName);

                //if (client != null) {
                //    client.update(Client.CHANGE_NAME);
                //}
            }

            FeatChunk::FeatChunk() : ChunkHandler(ID_FEAT, 0)
            {
            }

            void FeatChunk::HandleChunk(JdwpAgentPtr agent, JdwpPacketPtr packet)
            {
            }

            MPRQChunk::MPRQChunk() : ChunkHandler(ID_MPRQ, 0)
            {
            }

            void MPRQChunk::HandleChunk(JdwpAgentPtr agent, JdwpPacketPtr packet)
            {
            }

            JdwpClient::JdwpClient(IDevice * InDevice, int pid)
                : m_Device(InDevice)
                , m_PID(pid)
                , m_ReadBuffer(INIT_BUFFER_SIZE)
            {
                m_Status = Stat_Init;
                ReOpen();
                // create pass through
                m_Debugger = MakeShared<Debugger>(this, Debugger::NextPort());
            }
            
            JdwpClient::~JdwpClient()
            {
                
            }

            bool JdwpClient::SendHandShake()
            {
                int Sent = m_JdwpSock->Send(JdwpHandShake::HANDSHAKEBUFFER, 14);
                bool bSent = Sent == 14;
                if (bSent)
                {
                    m_Status = Stat_AwaitShake;
                }
                return bSent;
            }

            void JdwpClient::SendHelloCommands()
            {
                SendHello();
                //SendFEAT();
                //SendMPRQ();
            }

            void JdwpClient::SendHello()
            {
                JdwpPacket packet;
                auto Handler = MakeShared<HelloChunk>();
                Handler->Finish(packet);
                U8* Data = ChunkHandler::GetChunkData(packet);
                Write4BE(Data, 1);
                m_Requests[packet.GetId()] = Handler;
                SendPacket(packet);
            }

            void JdwpClient::SendFEAT()
            {
                JdwpPacket packet;
                auto Handler = MakeShared<FeatChunk>();
                Handler->Finish(packet);
                m_Requests[packet.GetId()] = Handler;
                SendPacket(packet);
            }

            void JdwpClient::SendMPRQ()
            {
                JdwpPacket packet;
                auto Handler = MakeShared<MPRQChunk>();
                Handler->Finish(packet);
                m_Requests[packet.GetId()] = Handler;
                SendPacket(packet);
            }

            void JdwpClient::SendPacket(JdwpPacket& Packet)
            {
                Packet.Write(m_JdwpSock.Get());
            }
            
            bool JdwpClient::ReadIn()
            {
                if (m_ReadBuffer.WrittenPosition() == m_ReadBuffer.Capacity()) 
                {
                    // Expand?
                }
                I32 recvLen = m_JdwpSock->Read(m_ReadBuffer);
                if (recvLen < 0) 
                {
                    int Error = m_JdwpSock->GetError();
#if _WIN32
                    if (Error == WSAEWOULDBLOCK)
#else
                    if (Error == EINTR || Error == EWOULDBLOCK || Error == EAGAIN)
#endif
                    {
                        //printf("Nonblocking recv.\n");
                    }
                    else
                    {
                        m_Status = Stat_Disconnected;
                    }
                }
                if (recvLen >= 0)
                {
                    if(recvLen > 0)
                        printf("Recved %d.\n", recvLen);
                    return true;
                }
                return false;
            }

            void JdwpClient::ReOpen()
            {
                m_JdwpSock = AdbHelper::CreatePassThroughConnection(m_Device->GetSerialId(), m_PID);
                m_JdwpSock->SetBlocking(false); // Non-Blocking IO
                m_JdwpSock->SetTcpNoDelay(true);
            }

            JdwpPacketPtr JdwpClient::GetPacket()
            {
                if (m_Status == Stat_AwaitShake)
                {
                    int Result = JdwpHandShake::FindHandShake(m_ReadBuffer);
                    switch (Result)
                    {
                    case JdwpHandShake::HANDSHAKE_GOOD:
                    {
                        m_Status = Stat_NeedDDMPkt;
                        JdwpHandShake::ConsumeHandShake(m_ReadBuffer);
                        SendHelloCommands();
                        return GetPacket();
                    }
                    case JdwpHandShake::HANDSHAKE_BAD:
                        printf("bad handshake.\n");
                        break;
                    case JdwpHandShake::HANDSHAKE_NOTYET:
                        printf("Still not found handshake.\n");
                        break;
                    }
                }
                else if (m_Status == Stat_NeedDDMPkt ||
                    m_Status == Stat_NotDDM ||
                    m_Status == Stat_Ready)
                {
                    return JdwpPacket::FindPacket(m_ReadBuffer);
                }
                else
                {
                    printf("Error State\n");
                    ReOpen();
                    bool HandShaked = SendHandShake();
                    if (!HandShaked)
                    {
                        printf("Handshake Failed.\n");
                    }
                    m_Status = Stat_AwaitShake;
                }

                return nullptr;
            }
            DebuggerPtr JdwpClient::GetDebugger()
            {
                return m_Debugger;
            }
            ChunkHandlerPtr JdwpClient::GetAndConsume(int Id)
            {
                auto Val = m_Requests.find(Id);
                if (Val != m_Requests.end())
                {
                    auto Ret = Val->second;
                    m_Requests.erase(Id);
                    return Ret;
                }
                return nullptr;
            }
            void JdwpClient::SetAppName(const String & InName)
            {
                m_AppName = InName;
            }
            int Debugger::DebuggerPort = DEBUGGER_PORT_BASE;
            int Debugger::NextPort()
            {
                return DebuggerPort++;
            }

            Debugger::Debugger(JdwpClient* client, int listenPort)
            {
                m_ListenChannel = AdbHelper::OpenServer(listenPort);
            }
            Debugger::~Debugger()
            {
            }
            void Debugger::Register(os::SocketSet & Set)
            {
                Set.Add(m_ListenChannel.Get(), this);
            }
            void Debugger::Accept(os::SocketSet & Set)
            {
                os::IpAddress ip("");
                os::Socket* C = m_ListenChannel->Accept(ip);
                if (C)
                {
                    C->SetBlocking(false);
                    Set.Add(C, this);
                    printf("Debugger Attached.\n");
                }
            }
            bool Debugger::SendHandShake()
            {
                return false;
            }
            bool Debugger::ReadIn()
            {
                return false;
            }
            JdwpPacketPtr Debugger::GetPacket()
            {
                return JdwpPacketPtr();
            }

            DeviceMonitor::DeviceMonitor()
            {
            }
            DeviceMonitor::~DeviceMonitor()
            {
            }
            void DeviceMonitor::SendHello(String const& serial, int pid)
            {
            }

}
    }
}
