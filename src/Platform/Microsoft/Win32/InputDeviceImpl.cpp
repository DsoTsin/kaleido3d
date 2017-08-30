#include "CoreMinimal.h"
#include "InputDeviceImpl.h"
#include "Base/Platform.h"
namespace k3d
{
	namespace InputDeviceImpl
	{
		bool isMouseButtonPressed(Mouse::Button button)
		{
			int vkey = 0;
			switch (button)
			{
			case Mouse::Left:     vkey = GetSystemMetrics(SM_SWAPBUTTON) ? VK_RBUTTON : VK_LBUTTON; break;
			case Mouse::Right:    vkey = GetSystemMetrics(SM_SWAPBUTTON) ? VK_LBUTTON : VK_RBUTTON; break;
			case Mouse::Middle:   vkey = VK_MBUTTON;  break;
			case Mouse::XButton1: vkey = VK_XBUTTON1; break;
			case Mouse::XButton2: vkey = VK_XBUTTON2; break;
			default:              vkey = 0;           break;
			}

			return (GetAsyncKeyState(vkey) & 0x8000) != 0;
		}

		Mouse::Vector2i getMousePosition()
		{
			POINT point;
			GetCursorPos(&point);
			return Mouse::Vector2i(point.x, point.y);
		}


		Mouse::Vector2i getMousePosition(const IWindow& relativeTo)
		{
			void* handle = relativeTo.GetHandle();
			if (handle)
			{
				POINT point;
				GetCursorPos(&point);
				::ScreenToClient((HWND)handle, &point);
				return Mouse::Vector2i(point.x, point.y);
			}
			else
			{
				return Mouse::Vector2i();
			}
		}


		void setMousePosition(const Mouse::Vector2i& position)
		{
			SetCursorPos(position[0], position[1]);
		}


		void setMousePosition(const Mouse::Vector2i& position, const IWindow& relativeTo)
		{
			void* handle = relativeTo.GetHandle();
			if (handle)
			{
				POINT point = { position[0], position[1] };
				::ClientToScreen((HWND)handle, &point);
				::SetCursorPos(point.x, point.y);
			}
		}

		bool isKeyPressed(Keyboard::Key key)
		{
			int vkey = 0;
			switch (key)
			{
			default:                   vkey = 0;             break;
			case Keyboard::A:          vkey = 'A';           break;
			case Keyboard::B:          vkey = 'B';           break;
			case Keyboard::C:          vkey = 'C';           break;
			case Keyboard::D:          vkey = 'D';           break;
			case Keyboard::E:          vkey = 'E';           break;
			case Keyboard::F:          vkey = 'F';           break;
			case Keyboard::G:          vkey = 'G';           break;
			case Keyboard::H:          vkey = 'H';           break;
			case Keyboard::I:          vkey = 'I';           break;
			case Keyboard::J:          vkey = 'J';           break;
			case Keyboard::K:          vkey = 'K';           break;
			case Keyboard::L:          vkey = 'L';           break;
			case Keyboard::M:          vkey = 'M';           break;
			case Keyboard::N:          vkey = 'N';           break;
			case Keyboard::O:          vkey = 'O';           break;
			case Keyboard::P:          vkey = 'P';           break;
			case Keyboard::Q:          vkey = 'Q';           break;
			case Keyboard::R:          vkey = 'R';           break;
			case Keyboard::S:          vkey = 'S';           break;
			case Keyboard::T:          vkey = 'T';           break;
			case Keyboard::U:          vkey = 'U';           break;
			case Keyboard::V:          vkey = 'V';           break;
			case Keyboard::W:          vkey = 'W';           break;
			case Keyboard::X:          vkey = 'X';           break;
			case Keyboard::Y:          vkey = 'Y';           break;
			case Keyboard::Z:          vkey = 'Z';           break;
			case Keyboard::Num0:       vkey = '0';           break;
			case Keyboard::Num1:       vkey = '1';           break;
			case Keyboard::Num2:       vkey = '2';           break;
			case Keyboard::Num3:       vkey = '3';           break;
			case Keyboard::Num4:       vkey = '4';           break;
			case Keyboard::Num5:       vkey = '5';           break;
			case Keyboard::Num6:       vkey = '6';           break;
			case Keyboard::Num7:       vkey = '7';           break;
			case Keyboard::Num8:       vkey = '8';           break;
			case Keyboard::Num9:       vkey = '9';           break;
			case Keyboard::Escape:     vkey = VK_ESCAPE;     break;
			case Keyboard::LControl:   vkey = VK_LCONTROL;   break;
			case Keyboard::LShift:     vkey = VK_LSHIFT;     break;
			case Keyboard::LAlt:       vkey = VK_LMENU;      break;
			case Keyboard::LSystem:    vkey = VK_LWIN;       break;
			case Keyboard::RControl:   vkey = VK_RCONTROL;   break;
			case Keyboard::RShift:     vkey = VK_RSHIFT;     break;
			case Keyboard::RAlt:       vkey = VK_RMENU;      break;
			case Keyboard::RSystem:    vkey = VK_RWIN;       break;
			case Keyboard::Menu:       vkey = VK_APPS;       break;
			case Keyboard::LBracket:   vkey = VK_OEM_4;      break;
			case Keyboard::RBracket:   vkey = VK_OEM_6;      break;
			case Keyboard::SemiColon:  vkey = VK_OEM_1;      break;
			case Keyboard::Comma:      vkey = VK_OEM_COMMA;  break;
			case Keyboard::Period:     vkey = VK_OEM_PERIOD; break;
			case Keyboard::Quote:      vkey = VK_OEM_7;      break;
			case Keyboard::Slash:      vkey = VK_OEM_2;      break;
			case Keyboard::BackSlash:  vkey = VK_OEM_5;      break;
			case Keyboard::Tilde:      vkey = VK_OEM_3;      break;
			case Keyboard::Equal:      vkey = VK_OEM_PLUS;   break;
			case Keyboard::Dash:       vkey = VK_OEM_MINUS;  break;
			case Keyboard::Space:      vkey = VK_SPACE;      break;
			case Keyboard::Return:     vkey = VK_RETURN;     break;
			case Keyboard::BackSpace:  vkey = VK_BACK;       break;
			case Keyboard::Tab:        vkey = VK_TAB;        break;
			case Keyboard::PageUp:     vkey = VK_PRIOR;      break;
			case Keyboard::PageDown:   vkey = VK_NEXT;       break;
			case Keyboard::End:        vkey = VK_END;        break;
			case Keyboard::Home:       vkey = VK_HOME;       break;
			case Keyboard::Insert:     vkey = VK_INSERT;     break;
			case Keyboard::Delete:     vkey = VK_DELETE;     break;
			case Keyboard::Add:        vkey = VK_ADD;        break;
			case Keyboard::Subtract:   vkey = VK_SUBTRACT;   break;
			case Keyboard::Multiply:   vkey = VK_MULTIPLY;   break;
			case Keyboard::Divide:     vkey = VK_DIVIDE;     break;
			case Keyboard::Left:       vkey = VK_LEFT;       break;
			case Keyboard::Right:      vkey = VK_RIGHT;      break;
			case Keyboard::Up:         vkey = VK_UP;         break;
			case Keyboard::Down:       vkey = VK_DOWN;       break;
			case Keyboard::Numpad0:    vkey = VK_NUMPAD0;    break;
			case Keyboard::Numpad1:    vkey = VK_NUMPAD1;    break;
			case Keyboard::Numpad2:    vkey = VK_NUMPAD2;    break;
			case Keyboard::Numpad3:    vkey = VK_NUMPAD3;    break;
			case Keyboard::Numpad4:    vkey = VK_NUMPAD4;    break;
			case Keyboard::Numpad5:    vkey = VK_NUMPAD5;    break;
			case Keyboard::Numpad6:    vkey = VK_NUMPAD6;    break;
			case Keyboard::Numpad7:    vkey = VK_NUMPAD7;    break;
			case Keyboard::Numpad8:    vkey = VK_NUMPAD8;    break;
			case Keyboard::Numpad9:    vkey = VK_NUMPAD9;    break;
			case Keyboard::F1:         vkey = VK_F1;         break;
			case Keyboard::F2:         vkey = VK_F2;         break;
			case Keyboard::F3:         vkey = VK_F3;         break;
			case Keyboard::F4:         vkey = VK_F4;         break;
			case Keyboard::F5:         vkey = VK_F5;         break;
			case Keyboard::F6:         vkey = VK_F6;         break;
			case Keyboard::F7:         vkey = VK_F7;         break;
			case Keyboard::F8:         vkey = VK_F8;         break;
			case Keyboard::F9:         vkey = VK_F9;         break;
			case Keyboard::F10:        vkey = VK_F10;        break;
			case Keyboard::F11:        vkey = VK_F11;        break;
			case Keyboard::F12:        vkey = VK_F12;        break;
			case Keyboard::F13:        vkey = VK_F13;        break;
			case Keyboard::F14:        vkey = VK_F14;        break;
			case Keyboard::F15:        vkey = VK_F16;        break;
			case Keyboard::Pause:      vkey = VK_PAUSE;      break;
			}

			return (GetAsyncKeyState(vkey) & 0x8000) != 0;
		}
	}

}