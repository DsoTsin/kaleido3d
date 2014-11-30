#pragma once
#include <Core/k3dWindow.h>

///
/// \brief The k3dEngineContext class
/// An AppWindow will own a k3dEngineContext
class k3dEngineContext {
public:

  /// \fn k3dEngineContext
  /// \param windev : SharedWindowPtr
  k3dEngineContext( SharedWindowPtr &windev ) : m_WinDev(windev) {}
  virtual ~k3dEngineContext() {}

  virtual void InitContext() = 0;
  virtual void DestroyContext() = 0;

  /// \fn MakeCurrent
  /// \brief MakeCurrent Sets current engine context
  virtual void MakeCurrent() = 0;

protected:

  SharedWindowPtr m_WinDev;
};
