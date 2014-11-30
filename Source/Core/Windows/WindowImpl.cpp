#include "Kaleido3D.h"
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
		Keyboard::Key virtualKeyCodeToSF(WPARAM key, LPARAM flags)
		{
			switch (key)
			{
				// Check the scancode to distinguish between left and right shift
			case VK_SHIFT:
			{
				static UINT lShift = MapVirtualKey(VK_LSHIFT, MAPVK_VK_TO_VSC);
				UINT scancode = static_cast<UINT>((flags & (0xFF << 16)) >> 16);
				return scancode == lShift ? Keyboard::LShift : Keyboard::RShift;
			}

			// Check the "extended" flag to distinguish between left and right alt
			case VK_MENU: return (HIWORD(flags) & KF_EXTENDED) ? Keyboard::RAlt : Keyboard::LAlt;

				// Check the "extended" flag to distinguish between left and right control
			case VK_CONTROL: return (HIWORD(flags) & KF_EXTENDED) ? Keyboard::RControl : Keyboard::LControl;

				// Other keys are reported properly
			case VK_LWIN:       return Keyboard::LSystem;
			case VK_RWIN:       return Keyboard::RSystem;
			case VK_APPS:       return Keyboard::Menu;
			case VK_OEM_1:      return Keyboard::SemiColon;
			case VK_OEM_2:      return Keyboard::Slash;
			case VK_OEM_PLUS:   return Keyboard::Equal;
			case VK_OEM_MINUS:  return Keyboard::Dash;
			case VK_OEM_4:      return Keyboard::LBracket;
			case VK_OEM_6:      return Keyboard::RBracket;
			case VK_OEM_COMMA:  return Keyboard::Comma;
			case VK_OEM_PERIOD: return Keyboard::Period;
			case VK_OEM_7:      return Keyboard::Quote;
			case VK_OEM_5:      return Keyboard::BackSlash;
			case VK_OEM_3:      return Keyboard::Tilde;
			case VK_ESCAPE:     return Keyboard::Escape;
			case VK_SPACE:      return Keyboard::Space;
			case VK_RETURN:     return Keyboard::Return;
			case VK_BACK:       return Keyboard::BackSpace;
			case VK_TAB:        return Keyboard::Tab;
			case VK_PRIOR:      return Keyboard::PageUp;
			case VK_NEXT:       return Keyboard::PageDown;
			case VK_END:        return Keyboard::End;
			case VK_HOME:       return Keyboard::Home;
			case VK_INSERT:     return Keyboard::Insert;
			case VK_DELETE:     return Keyboard::Delete;
			case VK_ADD:        return Keyboard::Add;
			case VK_SUBTRACT:   return Keyboard::Subtract;
			case VK_MULTIPLY:   return Keyboard::Multiply;
			case VK_DIVIDE:     return Keyboard::Divide;
			case VK_PAUSE:      return Keyboard::Pause;
			case VK_F1:         return Keyboard::F1;
			case VK_F2:         return Keyboard::F2;
			case VK_F3:         return Keyboard::F3;
			case VK_F4:         return Keyboard::F4;
			case VK_F5:         return Keyboard::F5;
			case VK_F6:         return Keyboard::F6;
			case VK_F7:         return Keyboard::F7;
			case VK_F8:         return Keyboard::F8;
			case VK_F9:         return Keyboard::F9;
			case VK_F10:        return Keyboard::F10;
			case VK_F11:        return Keyboard::F11;
			case VK_F12:        return Keyboard::F12;
			case VK_F13:        return Keyboard::F13;
			case VK_F14:        return Keyboard::F14;
			case VK_F15:        return Keyboard::F15;
			case VK_LEFT:       return Keyboard::Left;
			case VK_RIGHT:      return Keyboard::Right;
			case VK_UP:         return Keyboard::Up;
			case VK_DOWN:       return Keyboard::Down;
			case VK_NUMPAD0:    return Keyboard::Numpad0;
			case VK_NUMPAD1:    return Keyboard::Numpad1;
			case VK_NUMPAD2:    return Keyboard::Numpad2;
			case VK_NUMPAD3:    return Keyboard::Numpad3;
			case VK_NUMPAD4:    return Keyboard::Numpad4;
			case VK_NUMPAD5:    return Keyboard::Numpad5;
			case VK_NUMPAD6:    return Keyboard::Numpad6;
			case VK_NUMPAD7:    return Keyboard::Numpad7;
			case VK_NUMPAD8:    return Keyboard::Numpad8;
			case VK_NUMPAD9:    return Keyboard::Numpad9;
			case 'A':           return Keyboard::A;
			case 'Z':           return Keyboard::Z;
			case 'E':           return Keyboard::E;
			case 'R':           return Keyboard::R;
			case 'T':           return Keyboard::T;
			case 'Y':           return Keyboard::Y;
			case 'U':           return Keyboard::U;
			case 'I':           return Keyboard::I;
			case 'O':           return Keyboard::O;
			case 'P':           return Keyboard::P;
			case 'Q':           return Keyboard::Q;
			case 'S':           return Keyboard::S;
			case 'D':           return Keyboard::D;
			case 'F':           return Keyboard::F;
			case 'G':           return Keyboard::G;
			case 'H':           return Keyboard::H;
			case 'J':           return Keyboard::J;
			case 'K':           return Keyboard::K;
			case 'L':           return Keyboard::L;
			case 'M':           return Keyboard::M;
			case 'W':           return Keyboard::W;
			case 'X':           return Keyboard::X;
			case 'C':           return Keyboard::C;
			case 'V':           return Keyboard::V;
			case 'B':           return Keyboard::B;
			case 'N':           return Keyboard::N;
			case '0':           return Keyboard::Num0;
			case '1':           return Keyboard::Num1;
			case '2':           return Keyboard::Num2;
			case '3':           return Keyboard::Num3;
			case '4':           return Keyboard::Num4;
			case '5':           return Keyboard::Num5;
			case '6':           return Keyboard::Num6;
			case '7':           return Keyboard::Num7;
			case '8':           return Keyboard::Num8;
			case '9':           return Keyboard::Num9;
			}

			return Keyboard::Unknown;
		}

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
				if (window->callback)
					return CallWindowProc(reinterpret_cast<WNDPROC>(window->callback), hwnd, msg, wParam, lParam);
			}

			if (msg == WM_CLOSE)
				return 0;

			if ((msg == WM_SYSCOMMAND) && (wParam == SC_KEYMENU))
				return 0;
			return DefWindowProc(hwnd, msg, wParam, lParam);
		}

		int WindowPrivate::Init() {
			if (Global::gWindowCount == 0)
				RegisterWindowClass();

			callback = NULL;
			// Create window
			RECT rc = { 0, 0, 640, 480 };
			::AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
			handle = CreateWindow(Global::gClassName, "Default Window Name", WS_OVERLAPPEDWINDOW,
				CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, NULL, NULL,
				GetModuleHandle(NULL), this);

			if (!handle) {
				DBG_LINE_WITH_LAST_ERROR("WindowPrivate","InitWindow failed!");
				return E_FAIL;
			}

			Global::gWindowCount++;
			::ShowWindow(handle, SW_NORMAL);

			return S_OK;
		}

		void WindowPrivate::SetCaption(const kchar * name) {
			assert(handle != nullptr && "handle cannot be nullptr");
			::SetWindowTextW(handle, name);
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


		void WindowPrivate::PushMessage(const Message & message)
		{
			m_MessageQueue.push(message);
		}

		bool WindowPrivate::PopMessage(Message & message, bool block)
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
			if (!callback)
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
			if (handle == NULL)
				return;

			switch (message)
			{
				// Destroy event
				case WM_DESTROY:
				{
					// Here we must cleanup resources !
					//cleanup();
					break;
				}
			/*
						// Set cursor event
						case WM_SETCURSOR:
						{
						// The mouse has moved, if the cursor is in our window we must refresh the cursor
						if (LOWORD(lParam) == HTCLIENT)
						::SetCursor(m_cursor);

						break;
						}
						*/
			// Close event
			case WM_CLOSE:
			{
				Message event;
				event.type = Message::Closed;
				PushMessage(event);
				break;
			}
			/*
						// Resize event
						case WM_SIZE:
						{
						// Consider only events triggered by a maximize or a un-maximize
						if (wParam != SIZE_MINIMIZED && !m_resizing && m_lastSize != getSize())
						{
						// Update the last handled size
						m_lastSize = getSize();

						// Push a resize event
						Message event;
						event.type = Message::Resized;
						event.size.width = m_lastSize.x;
						event.size.height = m_lastSize.y;
						PushMessage(event);
						}
						break;
						}

						// Start resizing
						case WM_ENTERSIZEMOVE:
						{
						m_resizing = true;
						break;
						}

						// Stop resizing
						case WM_EXITSIZEMOVE:
						{
						m_resizing = false;

						// Ignore cases where the window has only been moved
						if (m_lastSize != getSize())
						{
						// Update the last handled size
						m_lastSize = getSize();

						// Push a resize event
						Message event;
						event.type = Message::Resized;
						event.size.width = m_lastSize.x;
						event.size.height = m_lastSize.y;
						PushMessage(event);
						}
						break;
						}
						*/
			// The system request the min/max window size and position
			case WM_GETMINMAXINFO:
			{
				// We override the returned information to remove the default limit
				// (the OS doesn't allow windows bigger than the desktop by default)
				MINMAXINFO* info = reinterpret_cast<MINMAXINFO*>(lParam);
				info->ptMaxTrackSize.x = 50000;
				info->ptMaxTrackSize.y = 50000;
				break;
			}

			// Gain focus event
			case WM_SETFOCUS:
			{
				Message event;
				event.type = Message::GainedFocus;
				PushMessage(event);
				break;
			}

			// Lost focus event
			case WM_KILLFOCUS:
			{
				Message event;
				event.type = Message::LostFocus;
				PushMessage(event);
				break;
			}
			/*
						// Keydown event
						case WM_KEYDOWN:
						case WM_SYSKEYDOWN:
						{
						if (m_keyRepeatEnabled || ((HIWORD(lParam) & KF_REPEAT) == 0))
						{
						Message event;
						event.type = Message::KeyPressed;
						event.key.alt = HIWORD(GetAsyncKeyState(VK_MENU)) != 0;
						event.key.control = HIWORD(GetAsyncKeyState(VK_CONTROL)) != 0;
						event.key.shift = HIWORD(GetAsyncKeyState(VK_SHIFT)) != 0;
						event.key.system = HIWORD(GetAsyncKeyState(VK_LWIN)) || HIWORD(GetAsyncKeyState(VK_RWIN));
						event.key.code = virtualKeyCodeToSF(wParam, lParam);
						PushMessage(event);
						}
						break;
						}
						*/
			// Keyup event
			case WM_KEYUP:
			case WM_SYSKEYUP:
			{
				Message event;
				event.type = Message::KeyReleased;
				event.key.alt = HIWORD(GetAsyncKeyState(VK_MENU)) != 0;
				event.key.control = HIWORD(GetAsyncKeyState(VK_CONTROL)) != 0;
				event.key.shift = HIWORD(GetAsyncKeyState(VK_SHIFT)) != 0;
				event.key.system = HIWORD(GetAsyncKeyState(VK_LWIN)) || HIWORD(GetAsyncKeyState(VK_RWIN));
				event.key.code = virtualKeyCodeToSF(wParam, lParam);
				PushMessage(event);
				break;
			}

			// Mouse wheel event
			case WM_MOUSEWHEEL:
			{
				// Mouse position is in screen coordinates, convert it to window coordinates
				POINT position;
				position.x = static_cast<int16>(LOWORD(lParam));
				position.y = static_cast<int16>(HIWORD(lParam));
				::ScreenToClient(handle, &position);

				Message event;
				event.type = Message::MouseWheelMoved;
				event.mouseWheel.delta = static_cast<int16>(HIWORD(wParam)) / 120;
				event.mouseWheel.x = position.x;
				event.mouseWheel.y = position.y;
				PushMessage(event);
				break;
			}

			// Mouse left button down event
			case WM_LBUTTONDOWN:
			{
				Message event;
				event.type = Message::MouseButtonPressed;
				event.mouseButton.button = Mouse::Left;
				event.mouseButton.x = static_cast<int16>(LOWORD(lParam));
				event.mouseButton.y = static_cast<int16>(HIWORD(lParam));
				PushMessage(event);
				break;
			}

			// Mouse left button up event
			case WM_LBUTTONUP:
			{
				Message event;
				event.type = Message::MouseButtonReleased;
				event.mouseButton.button = Mouse::Left;
				event.mouseButton.x = static_cast<int16>(LOWORD(lParam));
				event.mouseButton.y = static_cast<int16>(HIWORD(lParam));
				PushMessage(event);
				break;
			}

			// Mouse right button down event
			case WM_RBUTTONDOWN:
			{
				Message event;
				event.type = Message::MouseButtonPressed;
				event.mouseButton.button = Mouse::Right;
				event.mouseButton.x = static_cast<int16>(LOWORD(lParam));
				event.mouseButton.y = static_cast<int16>(HIWORD(lParam));
				PushMessage(event);
				break;
			}

			// Mouse right button up event
			case WM_RBUTTONUP:
			{
				Message event;
				event.type = Message::MouseButtonReleased;
				event.mouseButton.button = Mouse::Right;
				event.mouseButton.x = static_cast<int16>(LOWORD(lParam));
				event.mouseButton.y = static_cast<int16>(HIWORD(lParam));
				PushMessage(event);
				break;
			}

			// Mouse wheel button down event
			case WM_MBUTTONDOWN:
			{
				Message event;
				event.type = Message::MouseButtonPressed;
				event.mouseButton.button = Mouse::Middle;
				event.mouseButton.x = static_cast<int16>(LOWORD(lParam));
				event.mouseButton.y = static_cast<int16>(HIWORD(lParam));
				PushMessage(event);
				break;
			}

			// Mouse wheel button up event
			case WM_MBUTTONUP:
			{
				Message event;
				event.type = Message::MouseButtonReleased;
				event.mouseButton.button = Mouse::Middle;
				event.mouseButton.x = static_cast<int16>(LOWORD(lParam));
				event.mouseButton.y = static_cast<int16>(HIWORD(lParam));
				PushMessage(event);
				break;
			}

			// Mouse X button down event
			case WM_XBUTTONDOWN:
			{
				Message event;
				event.type = Message::MouseButtonPressed;
				event.mouseButton.button = HIWORD(wParam) == XBUTTON1 ? Mouse::XButton1 : Mouse::XButton2;
				event.mouseButton.x = static_cast<int16>(LOWORD(lParam));
				event.mouseButton.y = static_cast<int16>(HIWORD(lParam));
				PushMessage(event);
				break;
			}

			// Mouse X button up event
			case WM_XBUTTONUP:
			{
				Message event;
				event.type = Message::MouseButtonReleased;
				event.mouseButton.button = HIWORD(wParam) == XBUTTON1 ? Mouse::XButton1 : Mouse::XButton2;
				event.mouseButton.x = static_cast<int16>(LOWORD(lParam));
				event.mouseButton.y = static_cast<int16>(HIWORD(lParam));
				PushMessage(event);
				break;
			}

			// Mouse move event
			case WM_MOUSEMOVE:
			{
				// Extract the mouse local coordinates
				int x = static_cast<int16>(LOWORD(lParam));
				int y = static_cast<int16>(HIWORD(lParam));

				// Get the client area of the window
				RECT area;
				GetClientRect(handle, &area);

				// Check the mouse position against the window
				if ((x < area.left) || (x > area.right) || (y < area.top) || (y > area.bottom))
				{
					// Mouse is outside

					// Release the mouse capture
					ReleaseCapture();

					// Generate a MouseLeft event
					Message event;
					event.type = Message::MouseLeft;
					PushMessage(event);
				}
				else
				{
					// Mouse is inside
					if (GetCapture() != handle)
					{
						// Mouse was previously outside the window

						// Capture the mouse
						SetCapture(handle);

						// Generate a MouseEntered event
						Message event;
						event.type = Message::MouseEntered;
						PushMessage(event);
					}

					// Generate a MouseMove event
					Message event;
					event.type = Message::MouseMoved;
					event.mouseMove.x = x;
					event.mouseMove.y = y;
					PushMessage(event);
					break;
				}
			}
			default:
				if (message >= WM_USER) 
				{
					//....
					break;
				}
			}
		}
	}
}