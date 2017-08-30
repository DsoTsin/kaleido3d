#include "Kaleido3D.h"
#include <array>
#include <Core/Window.h>

#include <wrl/client.h>
#include <wrl/wrappers/corewrappers.h>

#include <windows.system.display.h>
#include <windows.ui.core.h>


using namespace ABI::Windows::Foundation;
using namespace ABI::Windows::Graphics::Display;
using namespace ABI::Windows::System::Display;
using namespace ABI::Windows::UI::Core;
using namespace ABI::Windows::UI::Input;
using namespace ABI::Windows::UI::ViewManagement;
using namespace Microsoft::WRL;
using namespace Microsoft::WRL::Wrappers;

namespace k3d
{
    namespace UWP
    {
        class Window : public IWindow
        {
        public:
            ComPtr<ABI::Windows::UI::Core::ICoreWindow> m_WindowsHandle;
            ComPtr<ABI::Windows::System::Display::IDisplayRequest> m_DisplayRequest;
            Window(const char *windowName, int width, int height)
            {

            }

            void	SetWindowCaption(const char * name) override
            {

            }
            void	Show(WindowMode mode = WindowMode::NORMAL) override
            {

            }
            void	Resize(int width, int height) override
            {

            }
            void	Move(int x, int y) override
            {

            }
            bool	IsOpen() override
            {
                return true;
            }
            void*	GetHandle() const override
            {
                return nullptr;
            }

            bool	PollMessage(Message & messge) override
            {
                return false;
            }

            uint32 	Width() const override
            {
                return 0;
            }
            uint32 	Height() const override
            {
                return 0;
            }
        };
    }

    IWindow::Ptr MakePlatformWindow(const char *windowName, int width, int height)
    {
        return MakeShared<UWP::Window>(windowName, width, height);
    }
}