#pragma once
#include <Interface/IRenderer.h>

namespace k3d {

	class Window;

	struct GLDeviceFormat {
		enum PixelType {
			RGBA
		};

		PixelType	pixelType;
		int			colorBits;
		int			depthBits;
		int			stencilBits;
	};

	class OGLDevice {
	public:
		static OGLDevice * CreateGLDevice(Window * window, GLDeviceFormat const & fmt);

		void Destroy();

		void MakeCurrent();

	private:
		explicit	OGLDevice(Window *);

		void		chooseFormat(GLDeviceFormat const &);
		bool		create(GLDeviceFormat const & fmt);
		Window *	pHostedWin;
	};

	class OGLRenderer : public IRenderer {
	public:
		~OGLRenderer() override;
		void SwapBuffers() override;

		static OGLRenderer * CreateRenderer(OGLDevice *);

	private:
		OGLRenderer(OGLDevice *);

		OGLDevice * pDevice;
	};
}