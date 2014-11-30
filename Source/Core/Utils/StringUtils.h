#pragma once

#include <string>

namespace k3d {

	struct StringUtil {
		static void	K3D_API	CharToWchar(const char *chr, wchar_t *wchar, int size);
		static void	K3D_API	WCharToChar(const wchar_t *wchr, char *wchar, int size);

		static std::string K3D_API GenerateMD5(std::string const & source);
	};

	extern std::string		GenerateShaderCachePath(const char * szPath, const char * szEntryPoint, const char * szShaderModel);
	extern std::string		GetLastWin32Error();
}