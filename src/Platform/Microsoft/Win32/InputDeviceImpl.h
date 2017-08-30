#pragma once

namespace k3d 
{
	namespace InputDeviceImpl
	{
		extern bool					isMouseButtonPressed(Mouse::Button button);
		extern Mouse::Vector2i		getMousePosition();
		extern Mouse::Vector2i		getMousePosition(const IWindow& relativeTo);
		extern void					setMousePosition(const Mouse::Vector2i& position);
		extern void					setMousePosition(const Mouse::Vector2i& position, const IWindow& relativeTo);

		extern bool					isKeyPressed(Keyboard::Key key);
	}

}