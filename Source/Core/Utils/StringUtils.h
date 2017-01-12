#pragma once

#include "Config/Config.h"
#include <string>

namespace k3d
{
    class K3D_API StringUtil
    {
    public:
#if K3DPLATFORM_OS_WIN
		static void CharToWchar(const char *chr, wchar_t *wchar, int size);
		static void	WCharToChar(const wchar_t *wchr, char *wchar, int size);
#endif
		static std::string GenerateMD5(std::string const & source);
	};

#if K3DPLATFORM_OS_WIN
	extern K3D_API std::string		GenerateShaderCachePath(const char * szPath, const char * szEntryPoint, const char * szShaderModel);
	extern K3D_API std::string		GetLastWin32Error();
#endif
}
