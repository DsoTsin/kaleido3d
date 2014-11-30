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

		void SwapBuffers();

	private:
		explicit	OGLDevice(Window *);

		void		chooseFormat(GLDeviceFormat const &);
		bool		create(GLDeviceFormat const & fmt);
		Window *	pHostedWin;
	};

	class OGLRenderer : public IRenderer {
	public:
		~OGLRenderer() override;

		void PrepareFrame() override;

		// traverse the scene tree and render the elements
		void DrawOneFrame() override;

		// do postprocessing and swap buffers
		void EndOneFrame() override;

		void DrawMesh(IRenderMesh *) override;
		
		static OGLRenderer * CreateRenderer(OGLDevice *);

		void OnResize(int width, int height) override;
			 
	protected:
		OGLRenderer(OGLDevice *);

		void SwapBuffers();

	private:

		OGLDevice * pDevice;
	};
}