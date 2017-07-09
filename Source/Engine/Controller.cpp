#include "k3dController.h"
#include <Core/k3dDbg.h>
#include <Core/k3dApplication.h>

k3dController::k3dController()
{
}

k3dController::~k3dController()
{
}

void k3dController::SetCamera(std::shared_ptr<k3dCamera> &camptr)
{
  m_CameraPtr.swap(camptr);
}

void k3dController::HandleMessage(SharedAppMsgHandlerPtr msg)
{
  K3D_UNUSED(msg);
}

