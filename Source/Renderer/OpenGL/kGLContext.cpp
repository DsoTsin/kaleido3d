#include "kGLContext.h"
#include <Core/k3dDbg.h>
#include <Platform/GLFW/kFWindow.h>

//NativePFD PixFormat::DefaultPFD()
//{
//#ifdef K3DPLATFORM_OS_WIN

//  static NativePFD pfd;

//  ::ZeroMemory( &pfd, sizeof(pfd) );

//  pfd.nSize = sizeof(pfd);
//  pfd.nVersion = 1;
//  pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_GENERIC_ACCELERATED | PFD_DOUBLEBUFFER;
//  pfd.iPixelType = PFD_TYPE_RGBA;
//  pfd.cColorBits = 24;
//  pfd.cRedBits = 8;
//  pfd.cGreenBits = 8;
//  pfd.cBlueBits = 8;
//  pfd.cDepthBits = 32;
//  pfd.iLayerType = PFD_MAIN_PLANE;

//#elif defined(K3DPLATFORM_OS_LINUX)

//  static GLint pfd[] = { GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None };

//#endif
//  return pfd;
//}


//--------------------kGLContext------------------------//
kGLContext::kGLContext(SharedWindowPtr &windev)
  : k3dEngineContext(windev)
{
#if defined(K3DPLATFORM_OS_WIN) && defined(USE_GLFW)
  m_DC    = ::GetDC(std::static_pointer_cast<kFWindow>(m_WinDev)->GetId());
  m_GLRC  = NULL;
#endif
}

kGLContext::~kGLContext()
{
  //.....
}

void kGLContext::InitContext() {
#if defined(K3DPLATFORM_OS_WIN)
  if (!m_GLRC)
    m_GLRC = ::wglCreateContext( m_DC );
#endif
  kDebug("kGLContext::InitContext Succeeds.\n");
  MakeCurrent();
}

//kGLContext::kGLContext( const kGLContext *sharedCtx, bool shared )
//{
//#ifdef K3DPLATFORM_OS_WIN
//  m_GLRC = ::wglCreateContext( sharedCtx->m_DC );
//  if ( shared ){
//    ::wglShareLists( sharedCtx->m_GLRC, m_GLRC );
//  }
//#elif defined(K3DPLATFORM_OS_LINUX)

//  //  m_GLRC = ::glXCreateNewContext(m_Display, );

//#endif
//}

void kGLContext::DestroyContext() {
#ifdef K3DPLATFORM_OS_WIN
  ::wglMakeCurrent( NULL, NULL );
  if (!m_GLRC) ::wglDeleteContext( m_GLRC );
#elif defined(K3DPLATFORM_OS_LINUX)
  ::glXMakeCurrent(m_Display, None, NULL);
  ::glXDestroyContext(m_Display, m_GLRC);
#endif
}

void kGLContext::MakeCurrent() {
#ifdef K3DPLATFORM_OS_WIN
  assert(m_DC != NULL && m_GLRC != NULL);
  ::wglMakeCurrent( m_DC, m_GLRC );
#elif defined(K3DPLATFORM_OS_LINUX)
  ::glXMakeCurrent(m_Display, m_DC, m_GLRC);
#endif
}
