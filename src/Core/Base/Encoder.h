#pragma once 

#ifndef __k3d_Encoder_h__
#define __k3d_Encoder_h__

/* Include MD5, SHA1 */
namespace k3d
{
    class K3D_CORE_API MD5
    {
    public:
        MD5();
        MD5(const void* input, size_t length);
        void update(const void* input, size_t length);

        void Update(const String& str);

        String Str();

        const U8* digest();
        void reset();

    private:
        void update(const U8* input, size_t length);
        void final();
        void transform(const U8 block[64]);
        void encode(const U32* input, U8* output, size_t length);
        void decode(const U8* input, U32* output, size_t length);

        /* class uncopyable */
        MD5(const MD5&);
        MD5& operator=(const MD5&);

    private:
        U32 _state[4];   /* state (ABCD) */
        U32 _count[2];   /* number of bits, modulo 2^64 (low-order word first) */
        U8 _buffer[64];   /* input buffer */
        U8 _digest[16];   /* message digest */
        bool _finished;     /* calculate finished ? */

        static const U8 PADDING[64];  /* padding for calculate */
        static const char HEX[16];
        enum { BUFFER_SIZE = 1024 };
    };

    class K3D_CORE_API SHA1
    {
    public:

        SHA1();
        virtual ~SHA1();

        /*
        *  Re-initialize the class
        */
        void Reset();

        /*
        *  Returns the message digest
        */
        bool Result(unsigned *message_digest_array);

        /*
        *  Provide input to SHA1
        */
        void Input(const unsigned char *message_array,
            unsigned            length);
        void Input(const char  *message_array,
            unsigned    length);
        void Input(unsigned char message_element);
        void Input(char message_element);
        SHA1& operator<<(const char *message_array);
        SHA1& operator<<(const unsigned char *message_array);
        SHA1& operator<<(const char message_element);
        SHA1& operator<<(const unsigned char message_element);

    private:

        /*
        *  Process the next 512 bits of the message
        */
        void ProcessMessageBlock();

        /*
        *  Pads the current message block to 512 bits
        */
        void PadMessage();

        /*
        *  Performs a circular left shift operation
        */
        inline unsigned CircularShift(int bits, unsigned word);

        unsigned H[5];                      // Message digest buffers

        unsigned Length_Low;                // Message length in bits
        unsigned Length_High;               // Message length in bits

        unsigned char Message_Block[64];    // 512-bit message blocks
        int Message_Block_Index;            // Index into message block array

        bool Computed;                      // Is the digest computed?
        bool Corrupted;                     // Is the message digest corruped?

    };

}

#endif