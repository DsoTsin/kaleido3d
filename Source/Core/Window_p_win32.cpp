#include <Config/OSHeaders.h>

#include "Window_p_win32.h"
#include "Window.h"

#include <assert.h>

#include <strsafe.h>

TCHAR gClassName[256] = "Kaleido3D";

using namespace k3d;

#define OUTPUT_LAST_ERROR() \
LPVOID lpMsgBuf;\
DWORD dw = GetLastError();\
FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,\
	NULL, dw, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&lpMsgBuf, 0, NULL);\
OutputDebugString((LPCSTR)lpMsgBuf);\
LocalFree(lpMsgBuf);

//--------------------------------------------------------------------------------------
// Called every time the application receives a message
//--------------------------------------------------------------------------------------
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		EndPaint(hWnd, &ps);
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
}

//--------------------------------------------------------------------------------------
// Register class and create window
//--------------------------------------------------------------------------------------
HRESULT InitWindow(HINSTANCE hInstance, int nCmdShow, HWND & hWnd)
{
	// Register class
	WNDCLASSEX wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, NULL);
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = gClassName;
	wcex.hIconSm = LoadIcon(wcex.hInstance, NULL);
	if (!RegisterClassEx(&wcex)) {
		OUTPUT_LAST_ERROR();
		return E_FAIL;
	}
	// Create window
	RECT rc = { 0, 0, 640, 480 };
	AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
	hWnd = CreateWindowA(gClassName, "Default Window Name", WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, NULL, NULL, hInstance,
		NULL);
	if (!hWnd) {
		OUTPUT_LAST_ERROR();
		return E_FAIL;
	}
	::ShowWindow(hWnd, nCmdShow);

	return S_OK;
}


int WindowPrivate::Init() {
	return InitWindow(GetModuleHandle(NULL), SW_SHOW, this->handle);
}

void WindowPrivate::SetCaption(const char * name) {
	assert(handle != nullptr && "handle cannot be nullptr");
	::SetWindowTextA(handle, name);
}

void WindowPrivate::Show(WindowMode mode) {
	assert(handle != nullptr);
	switch (mode)
	{
	case k3d::WindowMode::FULLSCREEN:
		::ShowWindow(handle, SW_SHOWMAXIMIZED);
		break;
	case k3d::WindowMode::NORMAL:
	default:
		::ShowWindow(handle, SW_NORMAL);
		break;
	}
}

void WindowPrivate::Resize(int width, int height) {
	assert(handle != nullptr);
	RECT rect;
	GetWindowRect(handle, &rect);
	::SetWindowPos(handle, NULL, rect.left, rect.top, width, height, SWP_SHOWWINDOW);
}

void WindowPrivate::Move(int x, int y) {
	assert(handle != nullptr);
	RECT rect;
	GetWindowRect(handle, &rect);
	::SetWindowPos(handle, NULL, x, y, rect.right-rect.left, rect.bottom-rect.top, SWP_SHOWWINDOW);
}