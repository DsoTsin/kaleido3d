#pragma once

#include <string>

namespace k3d {
	void			CharToWchar(const char *chr, wchar_t *wchar, int size);
	std::wstring	PathToHash(const wchar_t * filePath);
}