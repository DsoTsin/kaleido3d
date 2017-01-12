#include "Kaleido3D.h"
#include "InputDevice.h"

#if K3DPLATFORM_OS_WIN
#include "../Platform/Windows/InputDeviceImpl.h"
#elif K3DPLATFORM_OS_ANDROID
#include "../Platform/Android/cpp/InputDeviceImpl.h"
#endif

namespace k3d
{
	bool Mouse::IsButtonPressed(Button button)
	{
#ifndef K3DPLATFORM_OS_MAC
		return InputDeviceImpl::isMouseButtonPressed(button);
#else
        return true;
#endif
	}

	Mouse::Vector2i Mouse::GetPosition()
    {
#ifndef K3DPLATFORM_OS_MAC
		return InputDeviceImpl::getMousePosition();
#else
        return Mouse::Vector2i();
#endif
	}

	Mouse::Vector2i Mouse::GetPosition(const IWindow& relativeTo)
    {
#ifndef K3DPLATFORM_OS_MAC
        return InputDeviceImpl::getMousePosition(relativeTo);
#else
        return Mouse::Vector2i();
#endif
	}

	void Mouse::SetPosition(const Mouse::Vector2i& position)
    {
#ifndef K3DPLATFORM_OS_MAC
        return InputDeviceImpl::setMousePosition(position);
#endif
	}
	
	void Mouse::SetPosition(const Mouse::Vector2i& position, const IWindow& relativeTo)
    {
#ifndef K3DPLATFORM_OS_MAC
		return InputDeviceImpl::setMousePosition(position, relativeTo);
#endif
	}

	bool Keyboard::IsKeyPressed(Key key)
    {
#ifndef K3DPLATFORM_OS_MAC
		return InputDeviceImpl::isKeyPressed(key);
#else
        return true;
#endif
	}

}
