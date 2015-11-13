#include "Kaleido3D.h"
#include "ModuleCore.h"
#include "LogUtil.h"
#include "Config/OSHeaders.h"
#include <string>

namespace k3d {
	namespace Core {

#if K3DPLATFORM_OS_WIN
		std::wstring s_ModulePath = L"";
		void GetModulePath(IN HMODULE hModule, IN OUT LPWSTR lpPathBuffer, IN DWORD nSize);
#endif

		App* Init(kString const & appName) {
            AssetManager::Get().Init();
            
#if K3DPLATFORM_OS_WIN
			if(s_ModulePath==L"") {
				WCHAR path[2048] = { 0 };
				GetModulePath(NULL, path, 2048);
				s_ModulePath = path;
			}
			AssetManager::Get().AddSearchPath(s_ModulePath.c_str());
#endif
            
            Log::Get();
            
            return nullptr;
		}

		void Log(LogLevel && level, const char * tag, const char *fmt, ...) {

		}

		::k3d::AssetManager & GetAssetManager() {
			AssetManager & manager = AssetManager::Get();
			return manager;
		}

#if K3DPLATFORM_OS_WIN
		void GetModulePath(IN HMODULE hModule, IN OUT LPWSTR lpPathBuffer, IN DWORD nSize) {
			wchar_t *p, *q;
			p = (wchar_t*)calloc(nSize, sizeof(char));
			GetModuleFileNameW(hModule, p, nSize);
			q = p;
			while (wcschr(q, '\\'))
			{
				q = wcschr(q, '\\');
				q++;
			}
			*--q = '\0';
			wcscpy_s(lpPathBuffer, nSize, p);
		}
        
		kString GetExecutablePath() {
			if (s_ModulePath.empty()) {
				WCHAR path[2048] = { 0 };
				GetModulePath(NULL, path, 2048);
				s_ModulePath = path;
            }
            return s_ModulePath;
//            NSString * pPath = [[NSBundle mainBundle] executablePath];
//            return [pPath utf8String];
        }
#endif

	}
}