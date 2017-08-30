#include "CoreMinimal.h"
#include "InputDevice.h"

#if K3DPLATFORM_OS_WIN
#include "../Platform/Microsoft/Win32/InputDeviceImpl.h"
#elif K3DPLATFORM_OS_ANDROID
#include "../Platform/Android/cpp/InputDeviceImpl.h"
#endif

namespace k3d
{
	bool Mouse::IsButtonPressed(Button button)
	{
#if K3DPLATFORM_OS_WIN || K3DPLATFORM_OS_ANDROID
		return InputDeviceImpl::isMouseButtonPressed(button);
#else
        return true;
#endif
	}

	Mouse::Vector2i Mouse::GetPosition()
    {
#if K3DPLATFORM_OS_WIN || K3DPLATFORM_OS_ANDROID
		return InputDeviceImpl::getMousePosition();
#else
        return Mouse::Vector2i();
#endif
	}

	Mouse::Vector2i Mouse::GetPosition(const IWindow& relativeTo)
    {
#if K3DPLATFORM_OS_WIN || K3DPLATFORM_OS_ANDROID
        return InputDeviceImpl::getMousePosition(relativeTo);
#else
        return Mouse::Vector2i();
#endif
	}

	void Mouse::SetPosition(const Mouse::Vector2i& position)
    {
#if K3DPLATFORM_OS_WIN || K3DPLATFORM_OS_ANDROID
        return InputDeviceImpl::setMousePosition(position);
#endif
	}
	
	void Mouse::SetPosition(const Mouse::Vector2i& position, const IWindow& relativeTo)
    {
#if K3DPLATFORM_OS_WIN || K3DPLATFORM_OS_ANDROID
		return InputDeviceImpl::setMousePosition(position, relativeTo);
#endif
	}

	bool Keyboard::IsKeyPressed(Key key)
    {
#if K3DPLATFORM_OS_WIN || K3DPLATFORM_OS_ANDROID
		return InputDeviceImpl::isKeyPressed(key);
#else
        return true;
#endif
	}

}
