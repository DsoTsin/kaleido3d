#pragma once 
#ifndef __Window_h__
#define __Window_h__

namespace k3d {

	class Message;

	enum class WindowMode {
		NORMAL,
		FULLSCREEN
	};

	namespace WindowImpl {
		class WindowPrivate;
	}

	class Window {
	public:
		Window();

		virtual ~Window();

		void	SetWindowCaption(const char * name);
		void	Show(WindowMode mode = WindowMode::NORMAL);
		void	Resize(int width, int height);
		void	Move(int x, int y);

		//implemented by platform
		void*	GetHandle();

		bool	PollMessage(Message & messge);


	private:
		Window & operator = (Window const &) = delete;

		WindowImpl::WindowPrivate *pipl;
	};

};
#endif