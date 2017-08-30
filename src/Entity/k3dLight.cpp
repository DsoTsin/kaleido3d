#include "k3dLight.h"

k3dLight::k3dLight()
  : k3dRenderNode( Node_Light )
{
}

k3dLight::~k3dLight()
{

}

bool k3dLight::LoadJson(Json::Value &lightNode)
{
  return true;
}

bool k3dLight::NodeInVolume(kMath::Vec3f center, float size)
{
//  if (m_NodeType == Node_LightLine)
//  {
//    float dist = PointToSegDistance(position, tar_node, center);

//    if (attEnd > size*1.73205)
//    {
//      if (dist < attEnd-size*1.73205)return 2; // fully
//    }

//    if (dist <= attEnd+size*1.73205) // radius = size * sqrt(3.0)
//      return 1; // partially

//    return 0;
//  }

  else if (m_NodeType == Node_LightSpot)
  {
    bool fully = true;

    for (int p=0; p<6; p++)
    {
      float dist = VolumeNormal[p].x * center.x +
          VolumeNormal[p].y * center.y +
          VolumeNormal[p].z * center.z + VolumeD[p];

      if ( dist <= -size*1.73205 )
      {
        return 0;
      }

      if (dist <= size*1.73205)
        fully = false;
    }

    if (fully)
      return 2;

    return 1;
  }
  // else LIGHT_OMNI

  float dist = (position-center).Length();

  if (attEnd > size*1.73205)
  {
    if (dist < attEnd-size*1.73205)return 2; // fully
  }

  if (dist <= attEnd+size*1.73205) // radius = size * sqrt(3.0)
    return 1; // partially

  return 0;
}

bool k3dLight::ObjetInVolume(kSObject *obj)
{
//  if (m_NodeType == Node_LightLine)
//  {
//    if (PointToSegDistance(position, tar_node, ob->anim_Center) <=
//        attEnd+ob->radius)
//      return true;

//    return false;
//  }
  else if (m_NodeType == Node_LightSpot)
  {
    for (int p=0; p<6; p++)
    {
      int iInCount = 8;
      int iPtIn = 1;

      for (int i=0; i<8; i++)
      {
        if (VolumeNormal[p].x * (ob->anim_BBoxCorners[i].x) +
            VolumeNormal[p].y * (ob->anim_BBoxCorners[i].y) +
            VolumeNormal[p].z * (ob->anim_BBoxCorners[i].z) + VolumeD[p] < 0)
        {
          //iPtIn = 0;
          iInCount--;
        }
      }

      if (iInCount == 0)
        return false;

    }

    return true;
  }
  // else LIGHT_OMNI
  if ((position - ob->anim_Center).Length() <= attEnd + ob->radius)
    return true;

  return false;
}
