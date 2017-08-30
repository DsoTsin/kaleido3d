#include "CoreMinimal.h"
#include "Encoder.h"

namespace k3d
{
/* Constants for MD5Transform routine. */
#define S11 7
#define S12 12
#define S13 17
#define S14 22
#define S21 5
#define S22 9
#define S23 14
#define S24 20
#define S31 4
#define S32 11
#define S33 16
#define S34 23
#define S41 6
#define S42 10
#define S43 15
#define S44 21


    /* F, G, H and I are basic MD5 functions.
    */
#define F(x, y, z) (((x) & (y)) | ((~x) & (z)))
#define G(x, y, z) (((x) & (z)) | ((y) & (~z)))
#define H(x, y, z) ((x) ^ (y) ^ (z))
#define I(x, y, z) ((y) ^ ((x) | (~z)))

    /* ROTATE_LEFT rotates x left n bits.
    */
#define ROTATE_LEFT(x, n) (((x) << (n)) | ((x) >> (32-(n))))

    /* FF, GG, HH, and II transformations for rounds 1, 2, 3, and 4.
    Rotation is separate from addition to prevent recomputation.
    */
#define FF(a, b, c, d, x, s, ac) { \
    (a) += F ((b), (c), (d)) + (x) + ac; \
    (a) = ROTATE_LEFT ((a), (s)); \
    (a) += (b); \
}
#define GG(a, b, c, d, x, s, ac) { \
    (a) += G ((b), (c), (d)) + (x) + ac; \
    (a) = ROTATE_LEFT ((a), (s)); \
    (a) += (b); \
}
#define HH(a, b, c, d, x, s, ac) { \
    (a) += H ((b), (c), (d)) + (x) + ac; \
    (a) = ROTATE_LEFT ((a), (s)); \
    (a) += (b); \
}
#define II(a, b, c, d, x, s, ac) { \
    (a) += I ((b), (c), (d)) + (x) + ac; \
    (a) = ROTATE_LEFT ((a), (s)); \
    (a) += (b); \
}


    const U8 MD5::PADDING[64] = { 0x80 };
    const char MD5::HEX[16] = {
        '0', '1', '2', '3',
        '4', '5', '6', '7',
        '8', '9', 'a', 'b',
        'c', 'd', 'e', 'f'
    };


    /* Default construct. */
    MD5::MD5() {
        reset();
    }

    /* Construct a MD5 object with a input buffer. */
    MD5::MD5(const void* input, size_t length) {
        reset();
        update(input, length);
    }

    /* Return the message-digest */
    const U8* MD5::digest() {

        if (!_finished) {
            _finished = true;
            final();
        }
        return _digest;
    }

    /* Reset the calculate state */
    void MD5::reset() {

        _finished = false;
        /* reset number of bits. */
        _count[0] = _count[1] = 0;
        /* Load magic initialization constants. */
        _state[0] = 0x67452301;
        _state[1] = 0xefcdab89;
        _state[2] = 0x98badcfe;
        _state[3] = 0x10325476;
    }

    /* Updating the context with a input buffer. */
    void MD5::update(const void* input, size_t length) {
        update((const U8*)input, length);
    }

    /* MD5 block update operation. Continues an MD5 message-digest
    operation, processing another message block, and updating the
    context.
    */
    void MD5::update(const U8* input, size_t length) {

        U32 i, index, partLen;

        _finished = false;

        /* Compute number of bytes mod 64 */
        index = (U32)((_count[0] >> 3) & 0x3f);

        /* update number of bits */
        if ((_count[0] += ((U32)length << 3)) < ((U32)length << 3)) {
            ++_count[1];
        }
        _count[1] += ((U32)length >> 29);

        partLen = 64 - index;

        /* transform as many times as possible. */
        if (length >= partLen) {

            memcpy(&_buffer[index], input, partLen);
            transform(_buffer);

            for (i = partLen; i + 63 < length; i += 64) {
                transform(&input[i]);
            }
            index = 0;

        }
        else {
            i = 0;
        }

        /* Buffer remaining input */
        memcpy(&_buffer[index], &input[i], length - i);
    }

    /* MD5 finalization. Ends an MD5 message-_digest operation, writing the
    the message _digest and zeroizing the context.
    */
    void MD5::final() {

        U8 bits[8];
        U32 oldState[4];
        U32 oldCount[2];
        U32 index, padLen;

        /* Save current state and count. */
        memcpy(oldState, _state, 16);
        memcpy(oldCount, _count, 8);

        /* Save number of bits */
        encode(_count, bits, 8);

        /* Pad out to 56 mod 64. */
        index = (U32)((_count[0] >> 3) & 0x3f);
        padLen = (index < 56) ? (56 - index) : (120 - index);
        update(PADDING, padLen);

        /* Append length (before padding) */
        update(bits, 8);

        /* Store state in digest */
        encode(_state, _digest, 16);

        /* Restore current state and count. */
        memcpy(_state, oldState, 16);
        memcpy(_count, oldCount, 8);
    }

    /* MD5 basic transformation. Transforms _state based on block. */
    void MD5::transform(const U8 block[64]) {

        U32 a = _state[0], b = _state[1], c = _state[2], d = _state[3], x[16];

        decode(block, x, 64);

        /* Round 1 */
        FF(a, b, c, d, x[0], S11, 0xd76aa478); /* 1 */
        FF(d, a, b, c, x[1], S12, 0xe8c7b756); /* 2 */
        FF(c, d, a, b, x[2], S13, 0x242070db); /* 3 */
        FF(b, c, d, a, x[3], S14, 0xc1bdceee); /* 4 */
        FF(a, b, c, d, x[4], S11, 0xf57c0faf); /* 5 */
        FF(d, a, b, c, x[5], S12, 0x4787c62a); /* 6 */
        FF(c, d, a, b, x[6], S13, 0xa8304613); /* 7 */
        FF(b, c, d, a, x[7], S14, 0xfd469501); /* 8 */
        FF(a, b, c, d, x[8], S11, 0x698098d8); /* 9 */
        FF(d, a, b, c, x[9], S12, 0x8b44f7af); /* 10 */
        FF(c, d, a, b, x[10], S13, 0xffff5bb1); /* 11 */
        FF(b, c, d, a, x[11], S14, 0x895cd7be); /* 12 */
        FF(a, b, c, d, x[12], S11, 0x6b901122); /* 13 */
        FF(d, a, b, c, x[13], S12, 0xfd987193); /* 14 */
        FF(c, d, a, b, x[14], S13, 0xa679438e); /* 15 */
        FF(b, c, d, a, x[15], S14, 0x49b40821); /* 16 */

                                                /* Round 2 */
        GG(a, b, c, d, x[1], S21, 0xf61e2562); /* 17 */
        GG(d, a, b, c, x[6], S22, 0xc040b340); /* 18 */
        GG(c, d, a, b, x[11], S23, 0x265e5a51); /* 19 */
        GG(b, c, d, a, x[0], S24, 0xe9b6c7aa); /* 20 */
        GG(a, b, c, d, x[5], S21, 0xd62f105d); /* 21 */
        GG(d, a, b, c, x[10], S22, 0x2441453); /* 22 */
        GG(c, d, a, b, x[15], S23, 0xd8a1e681); /* 23 */
        GG(b, c, d, a, x[4], S24, 0xe7d3fbc8); /* 24 */
        GG(a, b, c, d, x[9], S21, 0x21e1cde6); /* 25 */
        GG(d, a, b, c, x[14], S22, 0xc33707d6); /* 26 */
        GG(c, d, a, b, x[3], S23, 0xf4d50d87); /* 27 */
        GG(b, c, d, a, x[8], S24, 0x455a14ed); /* 28 */
        GG(a, b, c, d, x[13], S21, 0xa9e3e905); /* 29 */
        GG(d, a, b, c, x[2], S22, 0xfcefa3f8); /* 30 */
        GG(c, d, a, b, x[7], S23, 0x676f02d9); /* 31 */
        GG(b, c, d, a, x[12], S24, 0x8d2a4c8a); /* 32 */

                                                /* Round 3 */
        HH(a, b, c, d, x[5], S31, 0xfffa3942); /* 33 */
        HH(d, a, b, c, x[8], S32, 0x8771f681); /* 34 */
        HH(c, d, a, b, x[11], S33, 0x6d9d6122); /* 35 */
        HH(b, c, d, a, x[14], S34, 0xfde5380c); /* 36 */
        HH(a, b, c, d, x[1], S31, 0xa4beea44); /* 37 */
        HH(d, a, b, c, x[4], S32, 0x4bdecfa9); /* 38 */
        HH(c, d, a, b, x[7], S33, 0xf6bb4b60); /* 39 */
        HH(b, c, d, a, x[10], S34, 0xbebfbc70); /* 40 */
        HH(a, b, c, d, x[13], S31, 0x289b7ec6); /* 41 */
        HH(d, a, b, c, x[0], S32, 0xeaa127fa); /* 42 */
        HH(c, d, a, b, x[3], S33, 0xd4ef3085); /* 43 */
        HH(b, c, d, a, x[6], S34, 0x4881d05); /* 44 */
        HH(a, b, c, d, x[9], S31, 0xd9d4d039); /* 45 */
        HH(d, a, b, c, x[12], S32, 0xe6db99e5); /* 46 */
        HH(c, d, a, b, x[15], S33, 0x1fa27cf8); /* 47 */
        HH(b, c, d, a, x[2], S34, 0xc4ac5665); /* 48 */

                                               /* Round 4 */
        II(a, b, c, d, x[0], S41, 0xf4292244); /* 49 */
        II(d, a, b, c, x[7], S42, 0x432aff97); /* 50 */
        II(c, d, a, b, x[14], S43, 0xab9423a7); /* 51 */
        II(b, c, d, a, x[5], S44, 0xfc93a039); /* 52 */
        II(a, b, c, d, x[12], S41, 0x655b59c3); /* 53 */
        II(d, a, b, c, x[3], S42, 0x8f0ccc92); /* 54 */
        II(c, d, a, b, x[10], S43, 0xffeff47d); /* 55 */
        II(b, c, d, a, x[1], S44, 0x85845dd1); /* 56 */
        II(a, b, c, d, x[8], S41, 0x6fa87e4f); /* 57 */
        II(d, a, b, c, x[15], S42, 0xfe2ce6e0); /* 58 */
        II(c, d, a, b, x[6], S43, 0xa3014314); /* 59 */
        II(b, c, d, a, x[13], S44, 0x4e0811a1); /* 60 */
        II(a, b, c, d, x[4], S41, 0xf7537e82); /* 61 */
        II(d, a, b, c, x[11], S42, 0xbd3af235); /* 62 */
        II(c, d, a, b, x[2], S43, 0x2ad7d2bb); /* 63 */
        II(b, c, d, a, x[9], S44, 0xeb86d391); /* 64 */

        _state[0] += a;
        _state[1] += b;
        _state[2] += c;
        _state[3] += d;
    }

    /* Encodes input (ulong) into output (U8). Assumes length is
    a multiple of 4.
    */
    void MD5::encode(const U32* input, U8* output, size_t length) {

        for (size_t i = 0, j = 0; j < length; ++i, j += 4) {
            output[j] = (U8)(input[i] & 0xff);
            output[j + 1] = (U8)((input[i] >> 8) & 0xff);
            output[j + 2] = (U8)((input[i] >> 16) & 0xff);
            output[j + 3] = (U8)((input[i] >> 24) & 0xff);
        }
    }

    /* Decodes input (U8) into output (ulong). Assumes length is
    a multiple of 4.
    */
    void MD5::decode(const U8* input, U32* output, size_t length) {

        for (size_t i = 0, j = 0; j < length; ++i, j += 4) {
            output[i] = ((U32)input[j]) | (((U32)input[j + 1]) << 8) |
                (((U32)input[j + 2]) << 16) | (((U32)input[j + 3]) << 24);
        }
    }

    k3d::String MD5::Str()
    {
        k3d::String str;
        str.Resize(32);
        for (size_t i = 0; i < 16; ++i) {
            int t = digest()[i];
            int a = t / 16;
            int b = t % 16;
            str += HEX[a];
            str += HEX[b];
        }
        return str;
    }

    void MD5::Update(const k3d::String & str)
    {
        update((const U8*)str.CStr(), str.Length());
    }

    /*
    *  SHA1
    *
    *  Description:
    *      This is the constructor for the sha1 class.
    *
    *  Parameters:
    *      None.
    *
    *  Returns:
    *      Nothing.
    *
    *  Comments:
    *
    */
    SHA1::SHA1()
    {
        Reset();
    }

    /*
    *  ~SHA1
    *
    *  Description:
    *      This is the destructor for the sha1 class
    *
    *  Parameters:
    *      None.
    *
    *  Returns:
    *      Nothing.
    *
    *  Comments:
    *
    */
    SHA1::~SHA1()
    {
        // The destructor does nothing
    }

    /*
    *  Reset
    *
    *  Description:
    *      This function will initialize the sha1 class member variables
    *      in preparation for computing a new message digest.
    *
    *  Parameters:
    *      None.
    *
    *  Returns:
    *      Nothing.
    *
    *  Comments:
    *
    */
    void SHA1::Reset()
    {
        Length_Low = 0;
        Length_High = 0;
        Message_Block_Index = 0;

        H[0] = 0x67452301;
        H[1] = 0xEFCDAB89;
        H[2] = 0x98BADCFE;
        H[3] = 0x10325476;
        H[4] = 0xC3D2E1F0;

        Computed = false;
        Corrupted = false;
    }

    /*
    *  Result
    *
    *  Description:
    *      This function will return the 160-bit message digest into the
    *      array provided.
    *
    *  Parameters:
    *      message_digest_array: [out]
    *          This is an array of five unsigned integers which will be filled
    *          with the message digest that has been computed.
    *
    *  Returns:
    *      True if successful, false if it failed.
    *
    *  Comments:
    *
    */
    bool SHA1::Result(unsigned *message_digest_array)
    {
        int i;                                  // Counter

        if (Corrupted)
        {
            return false;
        }

        if (!Computed)
        {
            PadMessage();
            Computed = true;
        }

        for (i = 0; i < 5; i++)
        {
            message_digest_array[i] = H[i];
        }

        return true;
    }

    /*
    *  Input
    *
    *  Description:
    *      This function accepts an array of octets as the next portion of
    *      the message.
    *
    *  Parameters:
    *      message_array: [in]
    *          An array of characters representing the next portion of the
    *          message.
    *
    *  Returns:
    *      Nothing.
    *
    *  Comments:
    *
    */
    void SHA1::Input(const unsigned char *message_array,
        unsigned            length)
    {
        if (!length)
        {
            return;
        }

        if (Computed || Corrupted)
        {
            Corrupted = true;
            return;
        }

        while (length-- && !Corrupted)
        {
            Message_Block[Message_Block_Index++] = (*message_array & 0xFF);

            Length_Low += 8;
            Length_Low &= 0xFFFFFFFF;               // Force it to 32 bits
            if (Length_Low == 0)
            {
                Length_High++;
                Length_High &= 0xFFFFFFFF;          // Force it to 32 bits
                if (Length_High == 0)
                {
                    Corrupted = true;               // Message is too long
                }
            }

            if (Message_Block_Index == 64)
            {
                ProcessMessageBlock();
            }

            message_array++;
        }
    }

    /*
    *  Input
    *
    *  Description:
    *      This function accepts an array of octets as the next portion of
    *      the message.
    *
    *  Parameters:
    *      message_array: [in]
    *          An array of characters representing the next portion of the
    *          message.
    *      length: [in]
    *          The length of the message_array
    *
    *  Returns:
    *      Nothing.
    *
    *  Comments:
    *
    */
    void SHA1::Input(const char  *message_array,
        unsigned    length)
    {
        Input((unsigned char *)message_array, length);
    }

    /*
    *  Input
    *
    *  Description:
    *      This function accepts a single octets as the next message element.
    *
    *  Parameters:
    *      message_element: [in]
    *          The next octet in the message.
    *
    *  Returns:
    *      Nothing.
    *
    *  Comments:
    *
    */
    void SHA1::Input(unsigned char message_element)
    {
        Input(&message_element, 1);
    }

    /*
    *  Input
    *
    *  Description:
    *      This function accepts a single octet as the next message element.
    *
    *  Parameters:
    *      message_element: [in]
    *          The next octet in the message.
    *
    *  Returns:
    *      Nothing.
    *
    *  Comments:
    *
    */
    void SHA1::Input(char message_element)
    {
        Input((unsigned char *)&message_element, 1);
    }

    /*
    *  operator<<
    *
    *  Description:
    *      This operator makes it convenient to provide character strings to
    *      the SHA1 object for processing.
    *
    *  Parameters:
    *      message_array: [in]
    *          The character array to take as input.
    *
    *  Returns:
    *      A reference to the SHA1 object.
    *
    *  Comments:
    *      Each character is assumed to hold 8 bits of information.
    *
    */
    SHA1& SHA1::operator<<(const char *message_array)
    {
        const char *p = message_array;

        while (*p)
        {
            Input(*p);
            p++;
        }

        return *this;
    }

    /*
    *  operator<<
    *
    *  Description:
    *      This operator makes it convenient to provide character strings to
    *      the SHA1 object for processing.
    *
    *  Parameters:
    *      message_array: [in]
    *          The character array to take as input.
    *
    *  Returns:
    *      A reference to the SHA1 object.
    *
    *  Comments:
    *      Each character is assumed to hold 8 bits of information.
    *
    */
    SHA1& SHA1::operator<<(const unsigned char *message_array)
    {
        const unsigned char *p = message_array;

        while (*p)
        {
            Input(*p);
            p++;
        }

        return *this;
    }

    /*
    *  operator<<
    *
    *  Description:
    *      This function provides the next octet in the message.
    *
    *  Parameters:
    *      message_element: [in]
    *          The next octet in the message
    *
    *  Returns:
    *      A reference to the SHA1 object.
    *
    *  Comments:
    *      The character is assumed to hold 8 bits of information.
    *
    */
    SHA1& SHA1::operator<<(const char message_element)
    {
        Input((unsigned char *)&message_element, 1);

        return *this;
    }

    /*
    *  operator<<
    *
    *  Description:
    *      This function provides the next octet in the message.
    *
    *  Parameters:
    *      message_element: [in]
    *          The next octet in the message
    *
    *  Returns:
    *      A reference to the SHA1 object.
    *
    *  Comments:
    *      The character is assumed to hold 8 bits of information.
    *
    */
    SHA1& SHA1::operator<<(const unsigned char message_element)
    {
        Input(&message_element, 1);

        return *this;
    }

    /*
    *  ProcessMessageBlock
    *
    *  Description:
    *      This function will process the next 512 bits of the message
    *      stored in the Message_Block array.
    *
    *  Parameters:
    *      None.
    *
    *  Returns:
    *      Nothing.
    *
    *  Comments:
    *      Many of the variable names in this function, especially the single
    *      character names, were used because those were the names used
    *      in the publication.
    *
    */
    void SHA1::ProcessMessageBlock()
    {
        const unsigned K[] = {               // Constants defined for SHA-1
            0x5A827999,
            0x6ED9EBA1,
            0x8F1BBCDC,
            0xCA62C1D6
        };
        int         t;                          // Loop counter
        unsigned    temp;                       // Temporary word value
        unsigned    W[80];                      // Word sequence
        unsigned    A, B, C, D, E;              // Word buffers

                                                /*
                                                *  Initialize the first 16 words in the array W
                                                */
        for (t = 0; t < 16; t++)
        {
            W[t] = ((unsigned)Message_Block[t * 4]) << 24;
            W[t] |= ((unsigned)Message_Block[t * 4 + 1]) << 16;
            W[t] |= ((unsigned)Message_Block[t * 4 + 2]) << 8;
            W[t] |= ((unsigned)Message_Block[t * 4 + 3]);
        }

        for (t = 16; t < 80; t++)
        {
            W[t] = CircularShift(1, W[t - 3] ^ W[t - 8] ^ W[t - 14] ^ W[t - 16]);
        }

        A = H[0];
        B = H[1];
        C = H[2];
        D = H[3];
        E = H[4];

        for (t = 0; t < 20; t++)
        {
            temp = CircularShift(5, A) + ((B & C) | ((~B) & D)) + E + W[t] + K[0];
            temp &= 0xFFFFFFFF;
            E = D;
            D = C;
            C = CircularShift(30, B);
            B = A;
            A = temp;
        }

        for (t = 20; t < 40; t++)
        {
            temp = CircularShift(5, A) + (B ^ C ^ D) + E + W[t] + K[1];
            temp &= 0xFFFFFFFF;
            E = D;
            D = C;
            C = CircularShift(30, B);
            B = A;
            A = temp;
        }

        for (t = 40; t < 60; t++)
        {
            temp = CircularShift(5, A) +
                ((B & C) | (B & D) | (C & D)) + E + W[t] + K[2];
            temp &= 0xFFFFFFFF;
            E = D;
            D = C;
            C = CircularShift(30, B);
            B = A;
            A = temp;
        }

        for (t = 60; t < 80; t++)
        {
            temp = CircularShift(5, A) + (B ^ C ^ D) + E + W[t] + K[3];
            temp &= 0xFFFFFFFF;
            E = D;
            D = C;
            C = CircularShift(30, B);
            B = A;
            A = temp;
        }

        H[0] = (H[0] + A) & 0xFFFFFFFF;
        H[1] = (H[1] + B) & 0xFFFFFFFF;
        H[2] = (H[2] + C) & 0xFFFFFFFF;
        H[3] = (H[3] + D) & 0xFFFFFFFF;
        H[4] = (H[4] + E) & 0xFFFFFFFF;

        Message_Block_Index = 0;
    }

    /*
    *  PadMessage
    *
    *  Description:
    *      According to the standard, the message must be padded to an even
    *      512 bits.  The first padding bit must be a '1'.  The last 64 bits
    *      represent the length of the original message.  All bits in between
    *      should be 0.  This function will pad the message according to those
    *      rules by filling the message_block array accordingly.  It will also
    *      call ProcessMessageBlock() appropriately.  When it returns, it
    *      can be assumed that the message digest has been computed.
    *
    *  Parameters:
    *      None.
    *
    *  Returns:
    *      Nothing.
    *
    *  Comments:
    *
    */
    void SHA1::PadMessage()
    {
        /*
        *  Check to see if the current message block is too small to hold
        *  the initial padding bits and length.  If so, we will pad the
        *  block, process it, and then continue padding into a second block.
        */
        if (Message_Block_Index > 55)
        {
            Message_Block[Message_Block_Index++] = 0x80;
            while (Message_Block_Index < 64)
            {
                Message_Block[Message_Block_Index++] = 0;
            }

            ProcessMessageBlock();

            while (Message_Block_Index < 56)
            {
                Message_Block[Message_Block_Index++] = 0;
            }
        }
        else
        {
            Message_Block[Message_Block_Index++] = 0x80;
            while (Message_Block_Index < 56)
            {
                Message_Block[Message_Block_Index++] = 0;
            }

        }

        /*
        *  Store the message length as the last 8 octets
        */
        Message_Block[56] = (Length_High >> 24) & 0xFF;
        Message_Block[57] = (Length_High >> 16) & 0xFF;
        Message_Block[58] = (Length_High >> 8) & 0xFF;
        Message_Block[59] = (Length_High) & 0xFF;
        Message_Block[60] = (Length_Low >> 24) & 0xFF;
        Message_Block[61] = (Length_Low >> 16) & 0xFF;
        Message_Block[62] = (Length_Low >> 8) & 0xFF;
        Message_Block[63] = (Length_Low) & 0xFF;

        ProcessMessageBlock();
    }


    /*
    *  CircularShift
    *
    *  Description:
    *      This member function will perform a circular shifting operation.
    *
    *  Parameters:
    *      bits: [in]
    *          The number of bits to shift (1-31)
    *      word: [in]
    *          The value to shift (assumes a 32-bit integer)
    *
    *  Returns:
    *      The shifted value.
    *
    *  Comments:
    *
    */
    unsigned SHA1::CircularShift(int bits, unsigned word)
    {
        return ((word << bits) & 0xFFFFFFFF) | ((word & 0xFFFFFFFF) >> (32 - bits));
    }
}