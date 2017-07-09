#ifndef __BASE64_H__
#define __BASE64_H__

#include <KTL/String.hpp>

#ifndef DISABLE_STL
#include <string>
#endif

namespace Base64 
{

#ifndef DISABLE_STL
	std::string K3D_API Encode(unsigned char const* , unsigned int len);
	std::string K3D_API Decode(std::string const& s);
#endif

  k3d::String K3D_API Encode(const char*s, uint64 len);
  k3d::String K3D_API Decode(k3d::String const& s);

}

#endif