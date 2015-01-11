#include "WindowImpl.h"
#include "../LogUtil.h"
#include "../Window.h"


namespace k3d 
{
	namespace Global {
		unsigned int				gWindowCount = 0;
		TCHAR						gClassName[256] = "Kaleido3D";
	}

	namespace WindowImpl
	{
		LRESULT WindowPrivate::WindowProc(HWND hwnd, UINT32 msg, WPARAM wParam, LPARAM lParam)
		{
			if (msg == WM_CREATE)
			{
				LONG_PTR window = (LONG_PTR)reinterpret_cast<CREATESTRUCT*>(lParam)->lpCreateParams;
				SetWindowLongPtr(hwnd, GWLP_USERDATA, window);
			}
			WindowPrivate* window = hwnd ? reinterpret_cast<WindowPrivate*>(GetWindowLongPtr(hwnd, GWLP_USERDATA)) : NULL;
			if (window)
			{
				window->ProcessMessage(msg, wParam, lParam);
				if (window->m_callback)
					return CallWindowProc(reinterpret_cast<WNDPROC>(window->m_callback), hwnd, msg, wParam, lParam);
			}

			if (msg == WM_CLOSE)
				return 0;

			if ((msg == WM_SYSCOMMAND) && (wParam == SC_KEYMENU))
				return 0;
			return DefWindowProc(hwnd, msg, wParam, lParam);
		}

		int WindowPrivate::Init() {
			if (Global::gWindowCount==0)
				RegisterWindowClass();

			// Create window
			RECT rc = { 0, 0, 640, 480 };
			::AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
			handle = CreateWindow(Global::gClassName, "Default Window Name", WS_OVERLAPPEDWINDOW,
				CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, NULL, NULL, 
				GetModuleHandle(NULL), this);

			if (!handle) {
				DBG_LINE_WITH_LAST_ERROR("InitWindow failed! ");
				return E_FAIL;
			}

			Global::gWindowCount++;
			::ShowWindow(handle, SW_NORMAL);

			return S_OK;
		}

		void WindowPrivate::SetCaption(const char * name) {
			assert(handle != nullptr && "handle cannot be nullptr");
			::SetWindowTextA(handle, name);
		}

		void WindowPrivate::Show(WindowMode mode) {
			assert(handle != nullptr);
			switch (mode)
			{
			case WindowMode::FULLSCREEN:
				::ShowWindow(handle, SW_SHOWMAXIMIZED);
				break;
			case WindowMode::NORMAL:
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
			::SetWindowPos(handle, NULL, x, y, rect.right - rect.left, rect.bottom - rect.top, SWP_SHOWWINDOW);
		}

		void WindowPrivate::RegisterWindowClass()
		{
			WNDCLASS windowClass;
			windowClass.style = 0;
			windowClass.lpfnWndProc = &WindowPrivate::WindowProc;
			windowClass.cbClsExtra = 0;
			windowClass.cbWndExtra = 0;
			windowClass.hInstance = GetModuleHandle(NULL);
			windowClass.hIcon = NULL;
			windowClass.hCursor = 0;
			windowClass.hbrBackground = 0;
			windowClass.lpszMenuName = NULL;
			windowClass.lpszClassName = Global::gClassName;
			RegisterClass(&windowClass);
		}


		void WindowPrivate::PushMessage(const MessageWin32 & message)
		{
			m_MessageQueue.push(message);
		}

		bool WindowPrivate::PopMessage(MessageWin32 & message, bool block)
		{
			if (m_MessageQueue.empty())
			{
				if (!block)
				{
					// Non-blocking mode: process events and continue
					//processJoystickEvents();
					ProcessMessage();
				}
				else
				{
					// Blocking mode: process events until one is triggered

					// Here we use a manual wait loop instead of the optimized
					// wait-event provided by the OS, so that we don't skip joystick
					// events (which require polling)
					while (m_MessageQueue.empty())
					{
						//processJoystickEvents();
						ProcessMessage();
						//sleep(milliseconds(10));
					}
				}
			}

			// Pop the first event of the queue, if it is not empty
			if (!m_MessageQueue.empty())
			{
				message = m_MessageQueue.front();
				m_MessageQueue.pop();
				return true;
			}

			return false;
		}

		void WindowPrivate::ProcessMessage()
		{
			if (!m_callback)
			{
				MSG message;
				while (::PeekMessage(&message, NULL, 0, 0, PM_REMOVE))
				{
					::TranslateMessage(&message);
					::DispatchMessage(&message);
				}
			}
		}

		void WindowPrivate::ProcessMessage(UINT message, WPARAM wParam, LPARAM lParam)
		{
			// pushmessage
		}
	}
}