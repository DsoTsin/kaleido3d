#pragma once 
#include "AdbHelper.h"

namespace k3d
{
    namespace mobi
    {
        class IDevice;
        namespace android
        {
            class Debugger;
            using DebuggerPtr = SharedPtr<Debugger>;


            class HelloChunk : public ChunkHandler//, public IJdwpInterceptor
            {
            public:
                HelloChunk();
                void HandleChunk(JdwpAgentPtr agent, JdwpPacketPtr packet) override;
            };

            class FeatChunk : public ChunkHandler
            {
            public:
                FeatChunk();
                void HandleChunk(JdwpAgentPtr agent, JdwpPacketPtr packet) override;
            };

            class MPRQChunk : public ChunkHandler
            {
            public:
                MPRQChunk();
                void HandleChunk(JdwpAgentPtr agent, JdwpPacketPtr packet) override;
            };

            using Requests = std::unordered_map<int, ChunkHandlerPtr>;

            class JdwpClient : public IJdwpAgent
            {
            public:
                enum EStatus
                {
                    Stat_Init,
                    Stat_NotJDWP,
                    Stat_AwaitShake,
                    Stat_NeedDDMPkt,
                    Stat_NotDDM,
                    Stat_Ready,
                    Stat_Error,
                    Stat_Disconnected
                };
                static const int INIT_BUFFER_SIZE = 2 * 1024;

                JdwpClient(IDevice* InDevice, int pid);
                ~JdwpClient() override;

                JdwpAgentType GetType() const
                {
                    return JdwpAgentType::Client;
                }
                SocketChannelPtr GetSock() override
                {
                    return m_JdwpSock;
                }

                bool SendHandShake();
                void SendHelloCommands();
                void SendHello();
                void SendFEAT();
                void SendMPRQ();

                void SendPacket(JdwpPacket& Packet);

                bool ReadIn();
                void ReOpen();

                JdwpPacketPtr GetPacket();

                DebuggerPtr GetDebugger();
                ChunkHandlerPtr GetAndConsume(int Id);

                void SetAppName(const String& InName);

                friend struct DeviceImpl;

            private:
                IDevice*            m_Device;
                int                 m_PID;
                SocketChannelPtr    m_JdwpSock;
                DebuggerPtr         m_Debugger;
                EStatus             m_Status;
                ByteBuffer          m_ReadBuffer;
                Requests            m_Requests;
                String              m_AppName;
            };

            using JdwpClientPtr = SharedPtr<JdwpClient>;

            class Debugger : public IJdwpAgent
            {
            public:
                static const int DEBUGGER_PORT_BASE = 8600;
                static int NextPort();

                Debugger(JdwpClient* client, int listenPort);
                ~Debugger() override;
                JdwpAgentType GetType() const
                {
                    return JdwpAgentType::Debugger;
                }
                SocketChannelPtr GetSock() override
                {
                    return m_Channel;
                }
                void Register(os::SocketSet& Set);
                void Accept(os::SocketSet & Set);
                bool SendHandShake();
                bool ReadIn();
                JdwpPacketPtr GetPacket();
            private:
                static int DebuggerPort;
                JdwpClient*   m_Client;
                SocketChannelPtr m_ListenChannel;
                SocketChannelPtr m_Channel; // accepted debugger client
            };

            class DeviceMonitor
            {
            public:
                DeviceMonitor();
                ~DeviceMonitor();
                    
                void SendHello(String const& serial, int pid);
            };
        }
    }
}