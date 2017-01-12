#include "Kaleido3D.h"
#include <Config/OSHeaders.h>
#include "StringUtils.h"
#include "MD5.h"

#if K3DPLATFORM_OS_WIN
	#include <tchar.h>
	#include <strsafe.h>
#endif


namespace k3d {

	std::string StringUtil::GenerateMD5(std::string const & source)
	{
		static MD5 md5;
		md5.update(source);
		return md5.toString();
	}

#ifdef K3DPLATFORM_OS_WIN
	
	void StringUtil::CharToWchar(const char *chr, wchar_t *wchar, int size)
	{
		::MultiByteToWideChar(CP_ACP, 0, chr, (int)strlen(chr) + 1, wchar, size / sizeof(wchar[0]));
	}


	void StringUtil::WCharToChar(const wchar_t *wchr, char *cchar, int size)
	{
		int nlength = (int)wcslen(wchr);
		int nbytes = WideCharToMultiByte(CP_ACP, 0, wchr, nlength, NULL, 0, NULL, NULL);
		if (nbytes>size)   nbytes = size;
		WideCharToMultiByte(0, 0, wchr, nlength, cchar, nbytes, NULL, NULL);
	}
	
	std::string GenerateShaderCachePath(const char * szPath, const char * szEntryPoint, const char * szShaderModel)
	{
		char catName[1024] = { 0 };
		::StringCbPrintfA(catName, 1024, "%s.%s.%s", szPath, szEntryPoint, szShaderModel);
		std::string newName = StringUtil::GenerateMD5(catName);
		std::string path(szPath);
		std::size_t posL = path.find_last_of("/");
		std::size_t posR = path.find_last_of("\\");

		std::size_t pos = std::string::npos;
		if (posL != std::string::npos && posR != std::string::npos) {
			pos = posL >= posR ? posL : posR;
		}
		else if (posL == std::string::npos) {
			pos = posR;
		}
		else if (posR == std::string::npos) {
			pos = posL;
		}

		if (pos != std::string::npos) {
			path = path.substr(0, pos);
			path.append("/").append(newName);
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
