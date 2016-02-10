#ifndef __WindowImpl_h__
#define __WindowImpl_h__

#include <queue>
#include "../Message.h"

namespace k3d
{
    namespace WindowImpl
    {
        class WindowPrivate
        {
        public:
            WindowPrivate();
            ~WindowPrivate();
            
            virtual int     Init();
            virtual void    SetCaption(const kchar * name);
            virtual void	Show(WindowMode mode);
            virtual void	Resize(int width, int height);
            virtual void	Move(int x, int y);
            
            virtual void	PushMessage(const Message & message);
            virtual bool	PopMessage(Message & message, bool block);
            virtual void	ProcessMessage();
            void *          handle;
        };
        
        extern WindowPrivate * makeNewWindowImpl();
    }
}

#endif