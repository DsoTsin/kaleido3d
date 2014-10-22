#ifndef __k3dGLRender_h__
#define __k3dGLRender_h__
#pragma once

#include <Renderer/k3dRender.h>

///
/// \brief The kGLRender class
///
class kGLRender : public k3dRender {
public:

  kGLRender();
  ~kGLRender();


  virtual void Init( SharedWindowPtr window ) override;
  virtual void Shutdown() override;

};

#endif
