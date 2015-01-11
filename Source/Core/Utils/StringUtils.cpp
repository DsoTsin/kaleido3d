#include "StringUtils.h"
#include <Config/OSHeaders.h>


#if K3DPLATFORM_OS_WIN
	#include <tchar.h>
	#include <strsafe.h>
#endif


namespace k3d {

#ifdef K3DPLATFORM_OS_WIN
	
	void StringUtil::CharToWchar(const char *chr, wchar_t *wchar, int size)
	{
		::MultiByteToWideChar(CP_ACP, 0, chr, (int)strlen(chr) + 1, wchar, size / sizeof(wchar[0]));
	}


	void StringUtil::WCharToChar(const wchar_t *wchr, char *cchar, int size)
	{
		int nlength = wcslen(wchr);
		int nbytes = WideCharToMultiByte(CP_ACP, 0, wchr, nlength, NULL, 0, NULL, NULL);
		if (nbytes>size)   nbytes = size;
		WideCharToMultiByte(0, 0, wchr, nlength, cchar, nbytes, NULL, NULL);
	}


	std::wstring PathToHash(const wchar_t * filePath)
	{
		std::wstring path(filePath);
		std::hash<std::wstring> hashFun;
		std::size_t code = hashFun(path);

		std::size_t posL = path.find_last_of(L"/");
		std::size_t posR = path.find_last_of(L"\\");

		std::size_t pos = std::wstring::npos;
		if (posL != std::wstring::npos && posR != std::wstring::npos) {
			pos = posL >= posR ? posL : posR;
		}
		else if (posL == std::wstring::npos) {
			pos = posR;
		}
		else if (posR == std::wstring::npos) {
			pos = posL;
		}
		wchar_t newName[128] = { 0 };
		::StringCbPrintfW(newName, 128, L"%d", code);
		if (pos != std::wstring::npos) {
			path = path.substr(0, pos);
			path.append(L"/").append(newName);
		}
		else {
			path = { newName };
		}
		return path;
	}

	std::string GetLastWin32Error()
	{
		LPVOID lpMsgBuf; 
		DWORD dw = GetLastError(); 
		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, 
						NULL, dw, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&lpMsgBuf, 0, NULL); 
		std::string lastError((LPCSTR)lpMsgBuf);
		LocalFree(lpMsgBuf);
		return lastError;
	}
#endif

}