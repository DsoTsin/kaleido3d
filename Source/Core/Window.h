#pragma once 
#ifndef __Window_h__
#define __Window_h__

namespace k3d {

	enum class WindowMode {
		NORMAL,
		FULLSCREEN
	};

	class Window {
	public:
		Window();

		virtual ~Window();

		void SetWindowCaption(const char * name);

		void Show(WindowMode mode = WindowMode::NORMAL);

		void Resize(int width, int height);

		void Move(int x, int y);

		//implemented by platform
		void* GetHandle();

	private:
		Window & operator = (Window const &) = delete;

		class WindowPrivate *pipl;
	};

};
#endif