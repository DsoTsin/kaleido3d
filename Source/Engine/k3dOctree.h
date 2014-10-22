#pragma once
#include <Math/kMath.hpp>
#include "k3dSceneObject.h"
#include "k3dFrustum.h"

class k3dSceneManager;
class k3dLight;

class k3dOctreeNode
{
public:
  kMath::Vec3f center;
  float length;
  k3dOctreeNode *children[2][2][2];
  k3dOctreeNode *parent;
  std::vector<kSObject*> objects;
  std::vector<kSObject*> splitableObjects;
  std::vector<k3dLight*> lights;

  void MakeChildren();

  k3dOctreeNode()
  {
    for (int i=0; i<2; i++)for (int o=0; o<2; o++)for (int p=0; p<2; p++)
      children[i][o][p] = 0;
  }

  ~k3dOctreeNode()
  {
    for (int i=0; i<2; i++)for (int o=0; o<2; o++)for (int p=0; p<2; p++)
    {
      if (children[i][o][p])delete children[i][o][p];
      children[i][o][p] = 0;
    }
  }
};

class k3dOctree
{
public:
  k3dSceneManager *scene;

  kMath::Vec3f minBound, maxBound;

  float len;

  int maxObjectsPerNode;

  k3dOctreeNode *root;

  void MakeOctree(k3dSceneManager *scene, int maxObjectsPerNode=10);

  void AddObject(kSObject *ob);

  void AddLight(k3dLight *sw);

  void AddToNode(k3dOctreeNode *node, kSObject *ob);
  void AddToNode(k3dOctreeNode *node, k3dLight *sw);

  void DeleteObject(kSObject *ob);

  void JoinNodes(k3dOctreeNode *node);

  void UpdateObject(kSObject *ob);

  void GetObjectsFrustum(k3dFrustum *frustum, std::vector<kSObject*> &visibleObjects,
                         std::vector<k3dOctreeNode*> &visibleNotEmptyNodes);
  void GetObjectsLight(k3dLight *sw, std::vector<kSObject*> &visibleObjects,
                       std::vector<k3dOctreeNode*> &visibleNotEmptyNodes);

  void AddObjectsFromNodeFrustum(std::vector<kSObject*> &obs, std::vector<k3dOctreeNode*> &visibleNotEmptyNodes,
                                 k3dOctreeNode *node, k3dFrustum *frustum, bool fullyInside);
  void AddObjectsFromNodeLight(std::vector<kSObject*> &obs, std::vector<k3dOctreeNode*> &visibleNotEmptyNodes,
                               k3dOctreeNode *node, k3dLight *sw, bool fullyInside);

  k3dOctree()
  {
    root = NULL;
  }

  ~k3dOctree()
  {
    if (root) {
      delete root;
      root = 0;
    }
  }
};
