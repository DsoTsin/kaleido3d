#pragma once

#include <string>

namespace k3d {

	struct StringUtil {
		static void			CharToWchar(const char *chr, wchar_t *wchar, int size);
		static void			WCharToChar(const wchar_t *wchr, char *wchar, int size);
	};

	extern std::wstring	PathToHash(const wchar_t * filePath);
	extern std::string		GetLastWin32Error();
}