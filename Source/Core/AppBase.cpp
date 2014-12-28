#include "AppBase.h"
#include "Engine/Engine.h"
#include <memory>

#if K3DPLATFORM_OS_WIN
#include "Win32Res/resource.h"
#endif

#define OUTPUT_LAST_ERROR() \
LPVOID lpMsgBuf;\
DWORD dw = GetLastError();\
FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,\
	NULL, dw, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&lpMsgBuf, 0, NULL);\
OutputDebugString((LPCSTR)lpMsgBuf);\
LocalFree(lpMsgBuf);

namespace k3d {

	// Gloabal Variables
	static std::unique_ptr<AppBase> gApp = nullptr;
#if K3DPLATFORM_OS_WIN
	static TCHAR		gClassName[256] = "Kaleido3D";
#endif
	AppBase::AppBase() : pEngine(nullptr) {
	}

	void AppBase::StartMessageLooping() {
		assert(pEngine != nullptr && "You should call \"SetEngine\" method!!");
#if K3DPLATFORM_OS_WIN
		MSG msg;
		ZeroMemory(&msg, sizeof(msg));
		while (WM_QUIT != msg.message)
		{
			if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			else {
				pEngine->DoOnDrawFrame();
			}
		}
#endif
	}

#if K3DPLATFORM_OS_WIN
	LRESULT CALLBACK AppBase::AppWndProc(HWND hwnd, UINT32 msg, WPARAM wParam, LPARAM lParam) {
		WinMsg _msg = { hwnd, msg, wParam, lParam };
		PAINTSTRUCT ps;
		HDC hdc;
		switch (msg)
		{
		case WM_PAINT:
			hdc = BeginPaint(hwnd, &ps);
			EndPaint(hwnd, &ps);
			break;
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
		default:
			if (gApp.get() != nullptr) {
				return gApp->processWinMessage(_msg);
			}
		}
		return 0;
	}

	std::unique_ptr<AppBase> & AppBase::CreateApplication(const HINSTANCE instance, const HICON icon) {
		if (!gApp) {
			std::unique_ptr<AppBase> ptr{ new AppBase(instance, icon) };
			gApp = std::move(ptr);
		}
		return gApp;
	}

	AppBase::AppBase(const HINSTANCE instance, const HICON icon) {
		WNDCLASSEX wcex;
		wcex.cbSize = sizeof(WNDCLASSEX);
		wcex.style = CS_HREDRAW | CS_VREDRAW;
		wcex.lpfnWndProc = AppBase::AppWndProc;
		wcex.cbClsExtra = 0;
		wcex.cbWndExtra = 0;
		wcex.hInstance = instance;
		wcex.hIcon = icon == NULL ? LoadIcon(instance, MAKEINTRESOURCE(IDI_IC_LAUNCHER)) : icon;
		wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
		wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
		wcex.lpszMenuName = NULL;
		wcex.lpszClassName = gClassName;
		wcex.hIconSm = LoadIcon(wcex.hInstance, NULL);
		// may not be elegate
		bool res = InitClass(wcex);
		assert(res == true);
	}

	bool AppBase::InitClass(WNDCLASSEX & wndClass) {
		if (!RegisterClassEx(&wndClass)) {
			OUTPUT_LAST_ERROR();
			return false;
		}
		return true;
	}

	LRESULT AppBase::processWinMessage(WinMsg & msg) {
		return DefWindowProc(msg.hwnd, msg.msg, msg.wParam, msg.lParam);
	}
#endif
}