#pragma once
#include <Core/kCoreObject.h>
#include "k3dCamera.h"
#include <Core/k3dInputEvent.h>
#include <Core/kMsgHandler.h>
//#include <dinput.h>
//#pragma comment(lib, "dinput8.lib")

class k3dCamera;


///
/// \brief The k3dController class handles user inputs and
/// updates the actors controlled, so it aggregates kApplicationMsgHandler
///
class k3dController
{
public:

  k3dController();
  virtual ~k3dController();

  ///
  /// \brief Set the camera controlled by controller
  /// \param[in, refCount] camptr
  ///
  virtual void SetCamera( std::shared_ptr<k3dCamera> & camptr );

  virtual void HandleMessage( SharedAppMsgHandlerPtr  msg );

protected:

  std::shared_ptr<k3dCamera> m_CameraPtr;
};

typedef std::shared_ptr<k3dController> SharedControllerPtr;
