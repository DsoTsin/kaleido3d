#include "k3dTerrain.h"

k3dTerrian::k3dTerrian()
{

}

k3dTerrian::~k3dTerrian()
{

}

k3dTerrian::RigidStaticPtr k3dTerrian::GetRigidPtr() const
{
  return m_RigidBody;
}
