#pragma once
#include <Core/CoreMinimal.h>

#ifndef _MSC_VER
#define PACK( __Declaration__ ) __Declaration__ __attribute__((__packed__))
#else
#define PACK( __Declaration__ ) __pragma( pack(push, 1) ) __Declaration__ __pragma( pack(pop) )
#endif

#include <unordered_map>

namespace k3d
{
    namespace mobi
    {
        namespace android
        {
            class IJdwpAgent;
            using JdwpAgentPtr = SharedPtr<IJdwpAgent>;

            inline void Write4BE(U8 *& ppDest, U32 Val)
            {
                *ppDest++ = (unsigned char)(Val >> 24);
                *ppDest++ = (unsigned char)(Val >> 16);
                *ppDest++ = (unsigned char)(Val >> 8);
                *ppDest = (unsigned char)(Val);
            }

            inline void Write2BE(U8 *& ppDest, U16 Val)
            {
                *ppDest++ = (unsigned char)(Val >> 8);
                *ppDest = (unsigned char)(Val);
            }

            inline void WriteBE(U8 *& ppDest, U8 Val)
            {
                *ppDest = (unsigned char)(Val);
            }

            inline String ReadBEString(U8 const** ppSrc, int StringLen)
            {
                String string(StringLen + 1, false);
                int i = 0;
                while (i < StringLen)
                {
                    *(*ppSrc)++;
                    string[i] = *(*ppSrc);
                    *(*ppSrc)++;
                    i++;
                }
                string[StringLen] = '\0';
                return string;
            }

            inline U32 Read4BE(U8 const** ppSrc)
            {
                U32 result = 0;
                result = *(*ppSrc)++ << 24;
                result |= *(*ppSrc)++ << 16;
                result |= *(*ppSrc)++ << 8;
                result |= *(*ppSrc)++;
                return result;
            }

            inline U16 Read2BE(U8 const** ppSrc)
            {
                U16 result = 0;
                result |= *(*ppSrc)++ << 8;
                result |= *(*ppSrc)++;
                return result;
            }

            inline U8 ReadBE(U8 const** ppSrc)
            {
                U8 result = 0;
                result |= *(*ppSrc)++;
                return result;
            }

            struct ByteBuffer
            {
                ByteBuffer(int Capacity)
                    : m_WrittenPosition(0)
                    , m_ReadPosition(0)
                    , m_Capacity(Capacity)
                {
                    if (Capacity > 0)
                    {
                        m_Data = new U8[Capacity];
                    }
                    else
                    {
                        m_Data = nullptr;
                    }
                }

                ~ByteBuffer()
                {
                    if (m_Data)
                    {
                        delete[] m_Data;
                        m_Data = nullptr;
                    }
                }

                int Capacity() const
                {
                    return m_Capacity;
                }

                char Get(int i) const
                {
                    return m_Data[i];
                }

                int WrittenPosition() const
                {
                    return m_WrittenPosition;
                }

                int ReadPosition() const
                {
                    return m_ReadPosition;
                }

                bool CanRead() const
                {
                    return m_ReadPosition < m_WrittenPosition;
                }

                const U8* ReadData() const
                {
                    return m_Data + m_ReadPosition;
                }
                // Write
                U8* CurrentData()
                {
                    return m_Data + m_WrittenPosition;
                }

                int Read(void* pDst, int Size)
                {
                    if (m_ReadPosition + Size < m_Capacity)
                    {
                        U8* ReadMem = m_Data + m_ReadPosition;
                        memcpy(pDst, ReadMem, Size);
                        m_ReadPosition += Size;
                        return Size;
                    }
                    return -1;
                }

                int Write(const void* pData, int Size)
                {
                    if (m_WrittenPosition + Size < m_Capacity)
                    {
                        U8* WritMem = m_Data + m_WrittenPosition;
                        memcpy(WritMem, pData, Size);
                        m_WrittenPosition += Size;
                        return Size;
                    }
                    return -1;
                }

                void Consume(int Size)
                {
                    m_WrittenPosition -= Size;
                    if (m_WrittenPosition < 0)
                    {
                        m_WrittenPosition = 0;
                    }
                    /*m_ReadPosition -= Size;
                    if (m_ReadPosition < 0)
                    {
                        m_ReadPosition = 0;
                    }*/
                }

                // SocketChannel can modify written position
                friend class SocketChannel;
                friend class JdwpPacket;
            private:
                U8 * m_Data;
                int m_WrittenPosition;
                int m_ReadPosition;
                int m_Capacity;
            };

            class JdwpPacket
            {
            public:
                JdwpPacket();
                virtual ~JdwpPacket();

                virtual bool Write(SocketChannel* socket);
                virtual void Read(ByteBuffer& buffer);

                U8* GetPayload() const 
                {
                    return m_PayloadData;
                }

                virtual void Dump();
                /*! Pack
                 @CmdSet 
                 @Cmd
                 */
                void Pack(int CmdSet, int Cmd);
                
                bool IsReply() const;
                bool IsEmpty() const;
                bool IsDDMPkt() const;
                bool IsError() const;
                int  GetId() const;
                int  GetLength() const;

                static int GetNextSerial();
                static SharedPtr<JdwpPacket> FindPacket(ByteBuffer& buff);

            public:
                // Command Packet
                // ----    ---- -     -           -
                // length  Id   Flags Command Set Command

                // Reply Packet
                // ----    ---- -     --
                // Length  Id   Flags Error Code
                // Flags is always '0x80'

                PACK(struct Header
                {
                    U32 Length;
                    U32 Id;
                    U8  Flags;
                    union
                    {
                        struct {
                            U8 CommandSet;
                            U8 Command;
                        };
                        U16 ErrorCode;
                    };
                });

                static const int HEADER_LEN = 11;
                static const int REPLY_PACKET = 0x80;

                static const int CHUNK_HEADER_LEN = 8; // 4-byte type, 4-byte len
                static const int DDMS_CMD_SET = 0xc7;
                static const int DDMS_CMD = 0x01;

                static int s_SerialId;/* = 0x40000000*/;

                friend class ChunkHandler;
            protected:
                U8 * m_PayloadData;
                int m_PayloadLength;
                Header m_Header;
            };
            using JdwpPacketPtr = SharedPtr<JdwpPacket>;

            class ChunkHandler/* : public JdwpPacket*/
            {
            public:
                static const int CHUNK_HEADER_LEN = 8;// 4-byte type, 4-byte len

                explicit ChunkHandler(int type, int ChunkLen = 0);
                virtual ~ChunkHandler() {}

                virtual void AgentReady(JdwpAgentPtr agent) {}
                virtual void HandleChunk(JdwpAgentPtr agent, JdwpPacketPtr packet) {}

                // Assemble Packet
                void Finish(JdwpPacket& packet);

                // Deserialize
                void HandlePacket(JdwpPacket const& packet);

                int Type() const;
                void Dump();

                static U8* GetChunkData(JdwpPacket& packet);

            private:
                I32 m_Type; // Chunk Type
                I32 m_ChunkLength; // Chunk Length
            };

            using ChunkHandlerPtr = SharedPtr<ChunkHandler>;

            class JdwpProtocol
            {

            };

            enum class JdwpAgentType
            {
                Debugger,
                Client,
            };

            class IJdwpAgent
            {
            public:
                virtual ~IJdwpAgent() {}

                virtual JdwpAgentType GetType() const = 0;
                virtual SharedPtr<SocketChannel> GetSock() = 0;

            protected:
                //   virtual void send(JdwpPacket, ) = 0;0
                JdwpProtocol protocol;
            };
        }
    }
}