#include "CoreMinimal.h"
#include <KTL/String.h>
#include <XPlatform/Window.h>
#include <XPlatform/Message.h>

namespace k3d
{
    namespace WindowImpl
    {
        class ProsperoWindow : public IWindow
        {
        public:
            ProsperoWindow(const char* windowName, int width, int height)
                : m_WindowName(windowName)
                , m_Width(width)
                , m_Height(height)
            {
            }

            void SetWindowCaption(const char* name) override
            {
                if (name) {
                    m_WindowName = name;
                }
            }

            void Show(WindowMode mode = WindowMode::NORMAL) override
            {
                K3D_UNUSED(mode);
            }

            void Resize(int width, int height) override
            {
                m_Width = width;
                m_Height = height;
            }

            void Move(int x, int y) override
            {
                K3D_UNUSED(x);
                K3D_UNUSED(y);
            }

            bool IsOpen() override
            {
                return m_IsOpen;
            }

            void* GetHandle() const override
            {
                return nullptr;
            }

            bool PollMessage(Message& message) override
            {
                K3D_UNUSED(message);
                return false;
            }

            U32 Width() const override
            {
                return static_cast<U32>(m_Width);
            }

            U32 Height() const override
            {
                return static_cast<U32>(m_Height);
            }

        private:
            String m_WindowName;
            int m_Width = 0;
            int m_Height = 0;
            bool m_IsOpen = true;
        };
    }

    IWindow::Ptr MakePlatformWindow(const char* windowName, int width, int height)
    {
        return MakeShared<WindowImpl::ProsperoWindow>(windowName, width, height);
    }
}
