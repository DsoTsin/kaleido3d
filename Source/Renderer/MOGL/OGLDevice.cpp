#include "OGLDevice.hpp"
#include "CommonGL.hpp"

#include <Core/Window.h>
#include <Config/OSHeaders.h>

#include <assert.h>

namespace k3d {
	// Global Variables
#if K3DPLATFORM_OS_WIN
	static HDC hDc = nullptr;
	static HGLRC hGLrc = nullptr;
#endif

	GLDevice* GLDevice::CreateGLDevice(Window* window, GLDeviceFormat const & fmt ) {
		if (window == nullptr)
			return nullptr;
		GLDevice * device = new GLDevice(window);
		if (!device->create(fmt))
			return nullptr;
		// Now initialize OpenGL Functions
		GLInitializer::InitAndCheck();

		return device;
	}

	GLDevice::GLDevice(Window * window)
		: pHostedWin(window)
	{}

	void GLDevice::chooseFormat(GLDeviceFormat const & fmt) {
#if K3DPLATFORM_OS_WIN
		assert(hDc != nullptr);
		PIXELFORMATDESCRIPTOR pfd;
		::ZeroMemory(&pfd, sizeof(pfd));
		pfd.nSize = sizeof(pfd);
		pfd.nVersion = 1;
		pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
		if (fmt.pixelType == GLDeviceFormat::PixelType::RGBA) {
			pfd.iPixelType = PFD_TYPE_RGBA;
		}
		pfd.cColorBits = (BYTE)fmt.colorBits;
		pfd.cDepthBits = (BYTE)fmt.depthBits;
		pfd.iLayerType = PFD_MAIN_PLANE;
		int format = ::ChoosePixelFormat(hDc, &pfd);
		::SetPixelFormat(hDc, format, &pfd);
#endif
	}

	void GLDevice::MakeCurrent() {
#if K3DPLATFORM_OS_WIN
		assert(hDc != nullptr && hGLrc != nullptr);
		::wglMakeCurrent(hDc, hGLrc);
#endif
	}

	void GLDevice::Destroy() {
#if K3DPLATFORM_OS_WIN
		assert(hDc != nullptr && hGLrc != nullptr);
		::wglDeleteContext(hGLrc);
#endif
	}

	bool GLDevice::create(GLDeviceFormat const & fmt) {
		assert(pHostedWin != nullptr);
		chooseFormat(fmt);

#if K3DPLATFORM_OS_WIN
		HWND hWnd = (HWND)pHostedWin->GetHandle();
		hDc = ::GetDC(hWnd);
		hGLrc= ::wglCreateContext(hDc);
		if (hWnd == nullptr || hDc == nullptr || hGLrc == nullptr)
			return false;
		::wglMakeCurrent(hDc, hGLrc);
#endif

		return true;
	}
}