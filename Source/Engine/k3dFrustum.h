#pragma once
#include <kMath.hpp>
#include "k3dSceneObject.h"
#include "k3dOctree.h"

class k3dLight;

class k3dFrustum {

public:
    k3dFrustum()
	{
		/*FILE *total_log = fopen("totallog.log", "a");
        fprintf(total_log, "k3dFrustum()\n");
		fclose(total_log);*/
	}

	// Call this every time the camera moves to update the frustum
	void CalculateFrustum();

	// This takes a 3D point and returns TRUE if it's inside of the frustum
	bool PointInFrustum(float x, float y, float z);

	// This takes a 3D point and a radius and	
	// return: 0 - not in frustum, 1 - partially in frustum, 2 - fully in frustum
	int SphereInFrustum(float x, float y, float z, float radius);

	// This takes the center and half the length of the cube.
	bool CubeInFrustum( float x, float y, float z, float size );

    bool BoxInFrustum(const kMath::Vec3f *vCorner);

    bool VolumeInFrustum(const kMath::Vec3f *vCorner);

    kMath::Vec3f frustumPoints[2][2][2];

    kMath::Mat4f Modelview;
    kMath::Mat4f Projection;
    kMath::Mat4f ModelviewProj;

    std::vector<kSObject*> visibleObjects;

    std::vector<k3dOctreeNode*> visibleNotEmptyNodes;

	void GenerateVisibleObjects();

    bool LightAffectsVisibleObjects(k3dLight *sw);

private:

	// This holds the A B C and D values for each side of our frustum.
	float m_Frustum[6][4];
};
