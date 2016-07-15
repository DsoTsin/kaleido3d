#ifndef __AndroidWindow_h__
#define __AndroidWindow_h__

#include <Kaleido3D.h>
#include <Core/Window.h>
#include <Core/LogUtil.h>
#include <queue>
#include "AndroidState.h"
#include <Core/App.h>
//#include <android/window.h>
#include <android/native_activity.h>

namespace k3d
{
    class AndroidWindow : public IWindow
    {
    public:

        explicit AndroidWindow(ANativeWindow *window)
        : m_pWindow(window)
        {

        }

        ~AndroidWindow() {}

        void SetWindowCaption(const kchar * name) override
        {

        }

        void Show(WindowMode mode = WindowMode::NORMAL) override
        {
//            if(mode == WindowMode::FULLSCREEN)
//            {
//                ANativeActivity_setWindowFlags(m_pActivity, AWINDOW_FLAG_FULLSCREEN,
//                                           AWINDOW_FLAG_FULLSCREEN);
//            }
        }

        void Resize(int width, int height) override
        {

        }

        void Move(int x, int y) override
        {

        }

        bool IsOpen() override
        {
            return true;
        }


        void* GetHandle() const override
        {
            return m_pWindow;
        }

        bool PollMessage(Message & messge) override
        {
            App* app = nullptr;
            int events;
            int ident = ALooper_pollAll(0, NULL, &events, (void**)&app);
            switch(ident)
            {
                case LOOPER_ID_INPUT:
                    app->ProcessInput(messge);
                    break;
                case LOOPER_ID_MAIN:
                    app->ProcessCmd();
                    break;
            }

            return ident>=0;
        }

        uint32 Width() const override
        {
            return (uint32)ANativeWindow_getWidth(m_pWindow);
        }

        uint32 Height() const override
        {
            return (uint32)ANativeWindow_getHeight(m_pWindow);
        }

    private:
        std::queue<Message> m_MessageQueue;
        ANativeWindow *     m_pWindow;
//        ANativeActivity* m_pActivity;
    };

    IWindow::Ptr MakePlatformWindow(const kchar *windowName, int width, int height)
    {
        return nullptr;
    }

    IWindow::Ptr MakeAndroidWindow(void* window)
    {
        KLOG(Info, kaleido3d::App, "[MakeAndroidWindow] file:%s line:%d", __FILE__, __LINE__);
        return IWindow::Ptr(new AndroidWindow( (ANativeWindow*) window ));
    }
}

#endif