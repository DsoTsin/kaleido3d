#include "Kaleido3D.h"
#include "Window.h"

#if K3DPLATFORM_OS_WIN
#include "Windows/WindowImpl.h"
#elif K3DPLATFORM_OS_IOS
#include "iOS/WindowImpl.h"
#elif K3DPLATFORM_OS_MAC
#include "MacOS/WindowImpl.h"
#endif

using namespace k3d;
using namespace k3d::WindowImpl;

Window::Window()
	: pipl(new WindowPrivate)
	, m_Width(0), m_Height(0)
{
	pipl->Init();
}

Window::Window(const kchar *windowName, int width, int height)
	: pipl(new WindowPrivate)
	, m_Width(width), m_Height(width)
{
	pipl->Init();
	pipl->SetCaption(windowName);
	pipl->Resize(width, height);
}

Window::~Window()
{
}

void * Window::GetHandle() const{
	assert(pipl != nullptr);
	return pipl->handle;
}

void Window::SetWindowCaption(const kchar * name) {
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
	m_Width = width;
	m_Height = height;
}

void Window::Move(int x, int y) {
	assert(pipl != nullptr);
	pipl->Move(x, y);
}

bool Window::IsOpen()
{
	return true;
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

uint32 Window::Width() const
{
	return m_Width;
}

uint32 Window::Height() const
{
	return m_Height;
}
