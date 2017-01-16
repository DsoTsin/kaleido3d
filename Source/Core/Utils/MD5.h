#pragma once 

#ifndef DISABLE_STL
#include <string>
#include <fstream>
#endif

#include <KTL/String.hpp>

/* Type define */
typedef unsigned char byte;
typedef unsigned int uint32;

/* MD5 declaration. */
class K3D_API MD5 
{
public:
	MD5();
	MD5(const void* input, size_t length);
	void update(const void* input, size_t length);

#ifndef DISABLE_STL
	MD5(const std::string& str);
	MD5(std::ifstream& in);
	void update(const std::string& str);
	void update(std::ifstream& in);
	std::string toString();
#endif
	void Update(const k3d::String& str);

	k3d::String Str();

	const byte* digest();
	void reset();

private:
	void update(const byte* input, size_t length);
	void final();
	void transform(const byte block[64]);
	void encode(const uint32* input, byte* output, size_t length);
	void decode(const byte* input, uint32* output, size_t length);

#ifndef DISABLE_STL
	std::string bytesToHexString(const byte* input, size_t length);
#endif

	/* class uncopyable */
	MD5(const MD5&);
	MD5& operator=(const MD5&);

private:
	uint32 _state[4];   /* state (ABCD) */
	uint32 _count[2];   /* number of bits, modulo 2^64 (low-order word first) */
	byte _buffer[64];   /* input buffer */
	byte _digest[16];   /* message digest */
	bool _finished;     /* calculate finished ? */

	static const byte PADDING[64];  /* padding for calculate */
	static const char HEX[16];
	enum { BUFFER_SIZE = 1024 };
};