#include "Jdwp.h"
#include "AdbHelper.h"
namespace k3d
{
    namespace mobi
    {
        namespace android
        {
            // Use GRPC to communicate with perfd
            // Need push perfd (realtime) or simpleperf (dump time duration)

            int JdwpPacket::s_SerialId = 0x40000000;

            JdwpPacket::JdwpPacket()
                : m_PayloadData(nullptr)
                , m_PayloadLength(0)
            {
                m_Header.Length = HEADER_LEN + m_PayloadLength;
                m_Header.Id = GetNextSerial();
                m_Header.Flags = 0;
                if (m_PayloadLength > 0)
                {
                    m_PayloadData = new U8[m_PayloadLength];
                }
            }

            JdwpPacket::~JdwpPacket()
            {
                if (m_PayloadData)
                {
                    delete[] m_PayloadData;
                    m_PayloadData = nullptr;
                }
            }

            bool JdwpPacket::Write(SocketChannel * socket)
            {
                U8* PacketData = new U8[m_Header.Length];
                U8* Mem = PacketData;
                Write4BE(Mem, m_Header.Length);
                Mem++;
                Write4BE(Mem, m_Header.Id);
                Mem++;
                WriteBE(Mem, m_Header.Flags);
                Mem++;
                WriteBE(Mem, m_Header.CommandSet);
                Mem++;
                WriteBE(Mem, m_Header.Command);
                Mem++;
                memcpy(Mem, m_PayloadData, m_PayloadLength);
                int Sent = socket->Write(PacketData, m_Header.Length);
                delete[] PacketData;
                bool AllSent = Sent == m_Header.Length;
                if (!AllSent)
                {
                    printf("Jdwp (%d) send failed.\n", m_Header.Id);
                }
                else
                {
                    Dump();
                }
                return AllSent;
            }

            void JdwpPacket::Read(ByteBuffer & buffer)
            {
                const U8* Data = buffer.ReadData();
                m_Header.Length = Read4BE(&Data);
                m_Header.Id = Read4BE(&Data);
                m_Header.Flags = ReadBE(&Data);
                m_Header.ErrorCode = Read2BE(&Data);
                buffer.m_ReadPosition += HEADER_LEN;
                m_PayloadLength = m_Header.Length - HEADER_LEN;
                if (m_PayloadData)
                {
                    delete[] m_PayloadData;
                }
                if (m_PayloadLength > 0)
                {
                    m_PayloadData = new U8[m_PayloadLength];
                    memcpy(m_PayloadData, buffer.ReadData(), m_PayloadLength);
                    buffer.m_ReadPosition += m_PayloadLength;
                }
            }

            void JdwpPacket::Dump()
            {
                if (m_PayloadData)
                {
                    const U8* Data = m_PayloadData;
                    int type = Read4BE(&Data);
                    int length = Read4BE(&Data);
                    char typeStr[5] = { 0 };
                    typeStr[0] = char((type >> 24) & 0x00ff);
                    typeStr[1] = char((type >> 16) & 0x00ff);
                    typeStr[2] = char((type >> 8) & 0x00ff);
                    typeStr[3] = char((type) & 0x00ff);
                    printf("\t\tJdwp: L:%d Id:%x F:%d CS:%x C:%x, CKT:%s CKL:%d.\n",
                        m_Header.Length, m_Header.Id, m_Header.Flags, m_Header.CommandSet, m_Header.Command,
                        typeStr,
                        length);
                }
                else
                {
                    printf("---Jdwp: L:%d Id:%x F:%d CS:%x C:%x.\n",
                        m_Header.Length, m_Header.Id, m_Header.Flags, m_Header.CommandSet, m_Header.Command);
                }
            }

            void JdwpPacket::Pack(int CmdSet, int Cmd)
            {
                m_Header.Length = HEADER_LEN + m_PayloadLength;
                //m_Header.Id = GetNextSerial();
                m_Header.Flags = 0;
                m_Header.CommandSet = CmdSet;
                m_Header.Command = Cmd;
            }

            bool JdwpPacket::IsReply() const
            {
                return (m_Header.Flags & REPLY_PACKET) != 0;
            }

            bool JdwpPacket::IsEmpty() const
            {
                return m_Header.Length == HEADER_LEN;
            }

            bool JdwpPacket::IsDDMPkt() const
            {
                return (m_Header.Flags & REPLY_PACKET) == 0 &&
                    m_Header.CommandSet == DDMS_CMD_SET &&
                    m_Header.Command == DDMS_CMD;
            }

            bool JdwpPacket::IsError() const
            {
                return IsReply() && m_Header.ErrorCode!=0;
            }

            int JdwpPacket::GetId() const
            {
                return m_Header.Id;
            }

            int JdwpPacket::GetLength() const
            {
                return m_Header.Length;
            }

            int JdwpPacket::GetNextSerial()
            {
                __intrinsics__::AtomicIncrement(&s_SerialId);
                return s_SerialId;
            }

            SharedPtr<JdwpPacket> JdwpPacket::FindPacket(ByteBuffer & buff)
            {
                if (buff.WrittenPosition() < sizeof(JdwpPacket::Header) || !buff.CanRead())
                    return nullptr;
                // parse header
                auto pkt = MakeShared<JdwpPacket>();
                pkt->Read(buff);
                return pkt;
            }

            ChunkHandler::ChunkHandler(int Type, int ChunkLen)
                : m_Type (Type)
                , m_ChunkLength(ChunkLen)
            {
            }

            void ChunkHandler::Finish(JdwpPacket& packet)
            {
                packet.m_PayloadLength = CHUNK_HEADER_LEN + m_ChunkLength;
                packet.m_Header.Length = CHUNK_HEADER_LEN + m_ChunkLength + JdwpPacket::HEADER_LEN;
                packet.Pack(JdwpPacket::DDMS_CMD_SET, JdwpPacket::DDMS_CMD);
                if (packet.m_PayloadData == nullptr)
                {
                    packet.m_PayloadData = new U8[packet.m_PayloadLength];
                }
                U8* Mem = packet.GetPayload();
                Write4BE(Mem, m_Type);
                Mem++;
                Write4BE(Mem, m_ChunkLength);
            }

            void ChunkHandler::HandlePacket(JdwpPacket const& packet)
            {
                const U8* Payload = packet.GetPayload();
                m_Type = Read4BE(&Payload);
                m_ChunkLength = Read4BE(&Payload);
            }

            int ChunkHandler::Type() const
            {
                return m_Type;
            }

            void ChunkHandler::Dump()
            {
            }

            U8* ChunkHandler::GetChunkData(JdwpPacket & packet)
            {
                return packet.m_PayloadData + CHUNK_HEADER_LEN;
            }

        }
    }
}
