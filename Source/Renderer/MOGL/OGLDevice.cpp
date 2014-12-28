#include "OGLRenderer.h"
#include "CommonGL.hpp"
#include <Config/OSHeaders.h>
#include <Core/Window.h>
#include <assert.h>

namespace k3d {

#if K3DPLATFORM_OS_WIN
	static HDC hDc = nullptr;
	static HGLRC hGLrc = nullptr;
#endif

	OGLDevice* OGLDevice::CreateGLDevice(Window* window, GLDeviceFormat const & fmt) {
		if (window == nullptr)
			return nullptr;
		OGLDevice * device = new OGLDevice(window);
		if (!device->create(fmt))
			return nullptr;
		// Now initialize OpenGL Functions
		GLInitializer::InitAndCheck();

		return device;
	}

	OGLDevice::OGLDevice(Window * window)
		: pHostedWin(window)
	{}

	void OGLDevice::chooseFormat(GLDeviceFormat const & fmt) {
#if K3DPLATFORM_OS_WIN
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
		pfd.cStencilBits = (BYTE)fmt.stencilBits;
		pfd.iLayerType = PFD_MAIN_PLANE;
		HWND hWnd = (HWND)(pHostedWin->GetHandle());
		hDc = ::GetDC(hWnd);
		assert(hDc != nullptr);
		int format = ::ChoosePixelFormat(hDc, &pfd);
		::SetPixelFormat(hDc, format, &pfd);
#endif
	}

	void OGLDevice::MakeCurrent() {
#if K3DPLATFORM_OS_WIN
		assert(hDc != nullptr && hGLrc != nullptr);
		::wglMakeCurrent(hDc, hGLrc);
#endif
	}

	void OGLDevice::Destroy() {
#if K3DPLATFORM_OS_WIN
		assert(hDc != nullptr && hGLrc != nullptr);
		::wglMakeCurrent(NULL, NULL);
		::wglDeleteContext(hGLrc);
		::ReleaseDC((HWND)pHostedWin->GetHandle(), hDc);
#endif
	}

	void OGLDevice::SwapBuffers() {
#if K3DPLATFORM_OS_WIN
		assert(hDc != nullptr);
		::SwapBuffers(hDc);
#endif
	}

	bool OGLDevice::create(GLDeviceFormat const & fmt) {
		assert(pHostedWin != nullptr);
		chooseFormat(fmt);

#if K3DPLATFORM_OS_WIN
		hGLrc = ::wglCreateContext(hDc);
		if (hDc == nullptr || hGLrc == nullptr)
			return false;
		::wglMakeCurrent(hDc, hGLrc);
#endif
		return true;
	}
}