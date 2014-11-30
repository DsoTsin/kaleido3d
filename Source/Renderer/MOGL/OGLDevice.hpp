#pragma once 

namespace k3d {

	class Window;

	struct GLDeviceFormat {
		enum PixelType {
			RGBA
		};

		PixelType	pixelType;
		int			colorBits;
		int			depthBits;
	};

	class GLDevice {
	public:
		static GLDevice * CreateGLDevice(Window * window, GLDeviceFormat const & fmt);

		void Destroy();

		void MakeCurrent();

	private:
		explicit	GLDevice(Window *);

		void		chooseFormat(GLDeviceFormat const &);
		bool		create(GLDeviceFormat const & fmt);
		Window *	pHostedWin;
	};
}