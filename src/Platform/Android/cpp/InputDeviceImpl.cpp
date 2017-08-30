#include "CoreMinimal.h"
#include "InputDeviceImpl.h"
//#include <android/input.h>

namespace k3d
{
	namespace InputDeviceImpl
	{
		bool isMouseButtonPressed(Mouse::Button button)
		{
			return true;
		}

		Mouse::Vector2i getMousePosition()
		{
			return Mouse::Vector2i();
		}


		Mouse::Vector2i getMousePosition(const IWindow& relativeTo)
		{
			return Mouse::Vector2i();
		}

		void setMousePosition(const Mouse::Vector2i& position)
		{

		}

		void setMousePosition(const Mouse::Vector2i& position, const IWindow& relativeTo)
		{
		}

		bool isKeyPressed(Keyboard::Key key)
		{
			return false;
		}
	}

}