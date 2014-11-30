#pragma once
#ifndef __Window_p_win32_h__
#define __Window_p_win32_h__
#include <Config/OSHeaders.h>

namespace k3d {

	enum class WindowMode;

	class WindowPrivate {
	public:
		WindowPrivate() = default;

		union {
			HWND handle;
		};

		int			Init();
		void		SetCaption(const char * name);
		void		Show(WindowMode mode);
		void		Resize(int width, int height);
		void		Move(int x, int y);
	};

};

extern int InitApp();

#endif