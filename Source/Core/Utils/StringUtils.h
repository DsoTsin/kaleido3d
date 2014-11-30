#pragma once

#include <string>

namespace k3d {

	struct StringUtil {
		static void			CharToWchar(const char *chr, wchar_t *wchar, int size);
		static void			WCharToChar(const wchar_t *wchr, char *wchar, int size);

		static std::string  GenerateMD5(std::string const & source);
	};

	extern std::string		GenerateShaderCachePath(const char * szPath, const char * szEntryPoint, const char * szShaderModel);
	extern std::string		GetLastWin32Error();
}