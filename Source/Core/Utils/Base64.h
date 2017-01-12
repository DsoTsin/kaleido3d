#ifndef __BASE64_H__
#define __BASE64_H__
#include <string>

namespace Base64 
{
	std::string K3D_API Encode(unsigned char const* , unsigned int len);
	std::string K3D_API Decode(std::string const& s);
}

#endif