#include "Kaleido3D.h"
#include "InputDevice.h"

#if K3DPLATFORM_OS_WIN
#include "../Platform/Windows/InputDeviceImpl.h"
#elif K3DPLATFORM_OS_ANDROID
#include "../Platform/Android/InputDeviceImpl.h"
#endif

namespace k3d
{
	bool Mouse::IsButtonPressed(Button button)
	{
		return InputDeviceImpl::isMouseButtonPressed(button);
	}

	Mouse::Vector2i Mouse::GetPosition()
	{
		return InputDeviceImpl::getMousePosition();
	}

	Mouse::Vector2i Mouse::GetPosition(const IWindow& relativeTo)
	{
		return InputDeviceImpl::getMousePosition(relativeTo);
	}

	void Mouse::SetPosition(const Mouse::Vector2i& position)
	{
		return InputDeviceImpl::setMousePosition(position);
	}
	
	void Mouse::SetPosition(const Mouse::Vector2i& position, const IWindow& relativeTo)
	{
		return InputDeviceImpl::setMousePosition(position, relativeTo);
	}

	bool Keyboard::IsKeyPressed(Key key)
	{
		return InputDeviceImpl::isKeyPressed(key);
	}

}