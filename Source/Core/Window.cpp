#include "Window.h"
#include <Config/Prerequisities.h>

#if K3DPLATFORM_OS_WIN
#include "Windows/WindowImpl.h"
#endif

using namespace k3d;
using namespace k3d::WindowImpl;

Window::Window()
	: pipl(new WindowPrivate)
{
	pipl->Init();
}

Window::~Window()
{
}

void * Window::GetHandle() {
	assert(pipl != nullptr);
	return pipl->handle;
}

void Window::SetWindowCaption(const char * name) {
	assert(name != nullptr && pipl != nullptr);
	pipl->SetCaption(name);
}

void Window::Show(WindowMode mode) {
	assert(pipl != nullptr);
	pipl->Show(mode);
}

void Window::Resize(int width, int height) {
	assert(pipl != nullptr);
	pipl->Resize(width, height);
}

void Window::Move(int x, int y) {
	assert(pipl != nullptr);
	pipl->Move(x, y);
}

bool Window::PollMessage(Message & message)
{
	if (pipl && pipl->PopMessage(message, false))
	{
		return true;
	}
	else
	{
		return false;
	}
}