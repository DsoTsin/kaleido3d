#ifndef __k3dWindow_h__
#define __k3dWindow_h__
#pragma once
#include <Engine/k3dEngineContext.h>
#include <Config/OSHeaders.h>

class kGLContext : public k3dEngineContext {
public:

  kGLContext( SharedWindowPtr &windev );
  ~kGLContext();

  void InitContext();
  void DestroyContext();
  void MakeCurrent();

private:

#if  defined(K3DPLATFORM_OS_WIN)
  HGLRC       m_GLRC;
  HDC         m_DC;
#elif defined(K3DPLATFORM_OS_LINUX)

  Display*        m_Display;
  XVisualInfo*    m_VisualInfo;

#endif
};
#endif
