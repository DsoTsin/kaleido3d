#include "k3dFrustum.h"
#include <Entity/k3dLight.h>

enum FrustumSide
{
    RIGHT	= 0,		// The RIGHT side of the frustum
    LEFT	= 1,		// The LEFT	 side of the frustum
    BOTTOM	= 2,		// The BOTTOM side of the frustum
    TOP		= 3,		// The TOP side of the frustum
    BACK	= 4,		// The BACK	side of the frustum
    FRONT	= 5			// The FRONT side of the frustum
};

// Like above, instead of saying a number for the ABC and D of the plane, we
// want to be more descriptive.
enum PlaneData
{
    A = 0,				// The X value of the plane's normal
    B = 1,				// The Y value of the plane's normal
    C = 2,				// The Z value of the plane's normal
    D = 3				// The distance the plane is from the origin
};

///////////////////////////////// NORMALIZE PLANE \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*
/////
/////	This normalizes a plane (A side) from a given frustum.
/////
///////////////////////////////// NORMALIZE PLANE \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*

void NormalizePlane(float frustum[6][4], int side)
{
    // Here we calculate the magnitude of the normal to the plane (point A B C)
    // Remember that (A, B, C) is that same thing as the normal's (X, Y, Z).
    // To calculate magnitude you use the equation:  magnitude = sqrt( x^2 + y^2 + z^2)
    float magnitude = (float)sqrt( frustum[side][A] * frustum[side][A] +
                                   frustum[side][B] * frustum[side][B] +
                                   frustum[side][C] * frustum[side][C] );

    // Then we divide the plane's values by it's magnitude.
    // This makes it easier to work with.
    frustum[side][A] /= magnitude;
    frustum[side][B] /= magnitude;
    frustum[side][C] /= magnitude;
    frustum[side][D] /= magnitude;
}

kMath::Vec3f Plane3Intersection(float *p1, float *p2, float *p3)
{
    kMath::Vec3f v = (kMath::Vec3f(p2[A], p2[B], p2[C]) * kMath::Vec3f(p3[A], p3[B], p3[C])) * (-p1[D]);
    v += (kMath::Vec3f(p3[A], p3[B], p3[C]) * kMath::Vec3f(p1[A], p1[B], p1[C])) * (-p2[D]);
    v += (kMath::Vec3f(p1[A], p1[B], p1[C]) * kMath::Vec3f(p2[A], p2[B], p2[C])) * (-p3[D]);

    v /= kMath::Vec3f(p1[A], p1[B], p1[C]) % (kMath::Vec3f(p2[A], p2[B], p2[C]) * kMath::Vec3f(p3[A], p3[B], p3[C]));

    return v;
}

///////////////////////////////// CALCULATE FRUSTUM \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*
/////
/////	This extracts our frustum from the projection and modelview matrix.
/////
///////////////////////////////// CALCULATE FRUSTUM \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*

void k3dFrustum::CalculateFrustum()
{
    float   proj[16];								// This will hold our projection matrix
    float   modl[16];								// This will hold our modelview matrix
    float   clip[16];								// This will hold the clipping planes

    // glGetFloatv() is used to extract information about our OpenGL world.
    // Below, we pass in GL_PROJECTION_MATRIX to abstract our projection matrix.
    // It then stores the matrix into an array of [16].
    glGetFloatv( GL_PROJECTION_MATRIX, proj );
    memcpy(Projection.m.m, proj, sizeof(float)*16);

    // By passing in GL_MODELVIEW_MATRIX, we can abstract our model view matrix.
    // This also stores it in an array of [16].
    glGetFloatv( GL_MODELVIEW_MATRIX, modl );
    memcpy(Modelview.m.m, modl, sizeof(float)*16);

    Projection.transpose();
    Modelview.transpose();
    ModelviewProj = Projection * Modelview;

    // Now that we have our modelview and projection matrix, if we combine these 2 matrices,
    // it will give us our clipping planes.  To combine 2 matrices, we multiply them.

    clip[ 0] = modl[ 0] * proj[ 0] + modl[ 1] * proj[ 4] + modl[ 2] * proj[ 8] + modl[ 3] * proj[12];
    clip[ 1] = modl[ 0] * proj[ 1] + modl[ 1] * proj[ 5] + modl[ 2] * proj[ 9] + modl[ 3] * proj[13];
    clip[ 2] = modl[ 0] * proj[ 2] + modl[ 1] * proj[ 6] + modl[ 2] * proj[10] + modl[ 3] * proj[14];
    clip[ 3] = modl[ 0] * proj[ 3] + modl[ 1] * proj[ 7] + modl[ 2] * proj[11] + modl[ 3] * proj[15];

    clip[ 4] = modl[ 4] * proj[ 0] + modl[ 5] * proj[ 4] + modl[ 6] * proj[ 8] + modl[ 7] * proj[12];
    clip[ 5] = modl[ 4] * proj[ 1] + modl[ 5] * proj[ 5] + modl[ 6] * proj[ 9] + modl[ 7] * proj[13];
    clip[ 6] = modl[ 4] * proj[ 2] + modl[ 5] * proj[ 6] + modl[ 6] * proj[10] + modl[ 7] * proj[14];
    clip[ 7] = modl[ 4] * proj[ 3] + modl[ 5] * proj[ 7] + modl[ 6] * proj[11] + modl[ 7] * proj[15];

    clip[ 8] = modl[ 8] * proj[ 0] + modl[ 9] * proj[ 4] + modl[10] * proj[ 8] + modl[11] * proj[12];
    clip[ 9] = modl[ 8] * proj[ 1] + modl[ 9] * proj[ 5] + modl[10] * proj[ 9] + modl[11] * proj[13];
    clip[10] = modl[ 8] * proj[ 2] + modl[ 9] * proj[ 6] + modl[10] * proj[10] + modl[11] * proj[14];
    clip[11] = modl[ 8] * proj[ 3] + modl[ 9] * proj[ 7] + modl[10] * proj[11] + modl[11] * proj[15];

    clip[12] = modl[12] * proj[ 0] + modl[13] * proj[ 4] + modl[14] * proj[ 8] + modl[15] * proj[12];
    clip[13] = modl[12] * proj[ 1] + modl[13] * proj[ 5] + modl[14] * proj[ 9] + modl[15] * proj[13];
    clip[14] = modl[12] * proj[ 2] + modl[13] * proj[ 6] + modl[14] * proj[10] + modl[15] * proj[14];
    clip[15] = modl[12] * proj[ 3] + modl[13] * proj[ 7] + modl[14] * proj[11] + modl[15] * proj[15];

    // Now we actually want to get the sides of the frustum.  To do this we take
    // the clipping planes we received above and extract the sides from them.

    // This will extract the RIGHT side of the frustum
    m_Frustum[RIGHT][A] = clip[ 3] - clip[ 0];
    m_Frustum[RIGHT][B] = clip[ 7] - clip[ 4];
    m_Frustum[RIGHT][C] = clip[11] - clip[ 8];
    m_Frustum[RIGHT][D] = clip[15] - clip[12];

    // Now that we have a normal (A,B,C) and a distance (D) to the plane,
    // we want to normalize that normal and distance.

    // Normalize the RIGHT side
    NormalizePlane(m_Frustum, RIGHT);

    // This will extract the LEFT side of the frustum
    m_Frustum[LEFT][A] = clip[ 3] + clip[ 0];
    m_Frustum[LEFT][B] = clip[ 7] + clip[ 4];
    m_Frustum[LEFT][C] = clip[11] + clip[ 8];
    m_Frustum[LEFT][D] = clip[15] + clip[12];

    // Normalize the LEFT side
    NormalizePlane(m_Frustum, LEFT);

    // This will extract the BOTTOM side of the frustum
    m_Frustum[BOTTOM][A] = clip[ 3] + clip[ 1];
    m_Frustum[BOTTOM][B] = clip[ 7] + clip[ 5];
    m_Frustum[BOTTOM][C] = clip[11] + clip[ 9];
    m_Frustum[BOTTOM][D] = clip[15] + clip[13];

    // Normalize the BOTTOM side
    NormalizePlane(m_Frustum, BOTTOM);

    // This will extract the TOP side of the frustum
    m_Frustum[TOP][A] = clip[ 3] - clip[ 1];
    m_Frustum[TOP][B] = clip[ 7] - clip[ 5];
    m_Frustum[TOP][C] = clip[11] - clip[ 9];
    m_Frustum[TOP][D] = clip[15] - clip[13];

    // Normalize the TOP side
    NormalizePlane(m_Frustum, TOP);

    // This will extract the BACK side of the frustum
    m_Frustum[BACK][A] = clip[ 3] - clip[ 2];
    m_Frustum[BACK][B] = clip[ 7] - clip[ 6];
    m_Frustum[BACK][C] = clip[11] - clip[10];
    m_Frustum[BACK][D] = clip[15] - clip[14];

    // Normalize the BACK side
    NormalizePlane(m_Frustum, BACK);

    // This will extract the FRONT side of the frustum
    m_Frustum[FRONT][A] = clip[ 3] + clip[ 2];
    m_Frustum[FRONT][B] = clip[ 7] + clip[ 6];
    m_Frustum[FRONT][C] = clip[11] + clip[10];
    m_Frustum[FRONT][D] = clip[15] + clip[14];

    // Normalize the FRONT side
    NormalizePlane(m_Frustum, FRONT);

    frustumPoints[0][0][0] = Plane3Intersection(m_Frustum[LEFT], m_Frustum[TOP], m_Frustum[BACK]);
    frustumPoints[1][0][0] = Plane3Intersection(m_Frustum[RIGHT], m_Frustum[TOP], m_Frustum[BACK]);
    frustumPoints[1][1][0] = Plane3Intersection(m_Frustum[RIGHT], m_Frustum[BOTTOM], m_Frustum[BACK]);
    frustumPoints[0][1][0] = Plane3Intersection(m_Frustum[LEFT], m_Frustum[BOTTOM], m_Frustum[BACK]);
    frustumPoints[0][0][1] = Plane3Intersection(m_Frustum[LEFT], m_Frustum[TOP], m_Frustum[FRONT]);
    frustumPoints[1][0][1] = Plane3Intersection(m_Frustum[RIGHT], m_Frustum[TOP], m_Frustum[FRONT]);
    frustumPoints[1][1][1] = Plane3Intersection(m_Frustum[RIGHT], m_Frustum[BOTTOM], m_Frustum[FRONT]);
    frustumPoints[0][1][1] = Plane3Intersection(m_Frustum[LEFT], m_Frustum[BOTTOM], m_Frustum[FRONT]);
}


// The code below will allow us to make checks within the frustum.  For example,
// if we want to see if a point, a sphere, or a cube lies inside of the frustum.
// Because all of our planes point INWARDS (The normals are all pointing inside the frustum)
// we then can assume that if a point is in FRONT of all of the planes, it's inside.

///////////////////////////////// POINT IN FRUSTUM \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*
/////
/////	This determines if a point is inside of the frustum
/////
///////////////////////////////// POINT IN FRUSTUM \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*

bool k3dFrustum::PointInFrustum( float x, float y, float z )
{
    // Go through all the sides of the frustum
    for(int i = 0; i < 6; i++ )
    {
        // Calculate the plane equation and check if the point is behind a side of the frustum
        if(m_Frustum[i][A] * x + m_Frustum[i][B] * y + m_Frustum[i][C] * z + m_Frustum[i][D] <= 0)
        {
            // The point was behind a side, so it ISN'T in the frustum
            return false;
        }
    }

    // The point was inside of the frustum (In front of ALL the sides of the frustum)
    return true;
}


///////////////////////////////// SPHERE IN FRUSTUM \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*
/////
/////	This determines if a sphere is inside of our frustum by it's center and radius.
/////
///////////////////////////////// SPHERE IN FRUSTUM \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*

int k3dFrustum::SphereInFrustum( float x, float y, float z, float radius )
{
    bool fully = true;

    // Go through all the sides of the frustum
    for(int i = 0; i < 6; i++ )
    {
        float dist = m_Frustum[i][A] * x + m_Frustum[i][B] * y + m_Frustum[i][C] * z + m_Frustum[i][D];

        // If the center of the sphere is farther away from the plane than the radius
        if ( dist <= -radius )
        {
            // The distance was greater than the radius so the sphere is outside of the frustum
            return 0;
        }

        if (dist <= radius)fully = false;
    }

    // The sphere was inside of the frustum!
    if (fully)return 2;

    return 1;
}


///////////////////////////////// CUBE IN FRUSTUM \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*
/////
/////	This determines if a cube is in or around our frustum by it's center and 1/2 it's length
/////
///////////////////////////////// CUBE IN FRUSTUM \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*

bool k3dFrustum::CubeInFrustum( float x, float y, float z, float size )
{
    // Basically, what is going on is, that we are given the center of the cube,
    // and half the length.  Think of it like a radius.  Then we checking each point
    // in the cube and seeing if it is inside the frustum.  If a point is found in front
    // of a side, then we skip to the next side.  If we get to a plane that does NOT have
    // a point in front of it, then it will return false.

    // *Note* - This will sometimes say that a cube is inside the frustum when it isn't.
    // This happens when all the corners of the bounding box are not behind any one plane.
    // This is rare and shouldn't effect the overall rendering speed.

    for(int i = 0; i < 6; i++ )
    {
        if(m_Frustum[i][A] * (x - size) + m_Frustum[i][B] * (y - size) + m_Frustum[i][C] * (z - size) + m_Frustum[i][D] > 0)
           continue;
        if(m_Frustum[i][A] * (x + size) + m_Frustum[i][B] * (y - size) + m_Frustum[i][C] * (z - size) + m_Frustum[i][D] > 0)
           continue;
        if(m_Frustum[i][A] * (x - size) + m_Frustum[i][B] * (y + size) + m_Frustum[i][C] * (z - size) + m_Frustum[i][D] > 0)
           continue;
        if(m_Frustum[i][A] * (x + size) + m_Frustum[i][B] * (y + size) + m_Frustum[i][C] * (z - size) + m_Frustum[i][D] > 0)
           continue;
        if(m_Frustum[i][A] * (x - size) + m_Frustum[i][B] * (y - size) + m_Frustum[i][C] * (z + size) + m_Frustum[i][D] > 0)
           continue;
        if(m_Frustum[i][A] * (x + size) + m_Frustum[i][B] * (y - size) + m_Frustum[i][C] * (z + size) + m_Frustum[i][D] > 0)
           continue;
        if(m_Frustum[i][A] * (x - size) + m_Frustum[i][B] * (y + size) + m_Frustum[i][C] * (z + size) + m_Frustum[i][D] > 0)
           continue;
        if(m_Frustum[i][A] * (x + size) + m_Frustum[i][B] * (y + size) + m_Frustum[i][C] * (z + size) + m_Frustum[i][D] > 0)
           continue;

        // If we get here, it isn't in the frustum
        return false;
    }

    // additional tests

    int out = 0;
    for (int i=0; i<2; i++)for (int o=0; o<2; o++)for (int p=0; p<2; p++)
        if (frustumPoints[i][o][p].x > x + size)out++;
    if (out == 8)return false;

    out = 0;
    for (int i=0; i<2; i++)for (int o=0; o<2; o++)for (int p=0; p<2; p++)
        if (frustumPoints[i][o][p].x < x - size)out++;
    if (out == 8)return false;

    out = 0;
    for (int i=0; i<2; i++)for (int o=0; o<2; o++)for (int p=0; p<2; p++)
        if (frustumPoints[i][o][p].y > y + size)out++;
    if (out == 8)return false;

    out = 0;
    for (int i=0; i<2; i++)for (int o=0; o<2; o++)for (int p=0; p<2; p++)
        if (frustumPoints[i][o][p].y < y - size)out++;
    if (out == 8)return false;

    out = 0;
    for (int i=0; i<2; i++)for (int o=0; o<2; o++)for (int p=0; p<2; p++)
        if (frustumPoints[i][o][p].z > z + size)out++;
    if (out == 8)return false;

    out = 0;
    for (int i=0; i<2; i++)for (int o=0; o<2; o++)for (int p=0; p<2; p++)
        if (frustumPoints[i][o][p].z < z - size)out++;
    if (out == 8)return false;

    return true;
}

bool k3dFrustum::BoxInFrustum(const kMath::Vec3f *vCorner)
{
    //int iTotalIn = 0;

    for (int p = 0; p < 6; ++p)
    {
        int iInCount = 8;
        int iPtIn = 1;

        for (int i = 0; i < 8; ++i)
        {
            if (m_Frustum[p][A] * (vCorner[i].x) + m_Frustum[p][B] * (vCorner[i].y) +
                m_Frustum[p][C] * (vCorner[i].z) + m_Frustum[p][D] < 0)
            {
                //iPtIn = 0;
                --iInCount;
            }
        }

        if (iInCount == 0)
            return false;

        //iTotalIn += iPtIn;
    }

    return true;

    /*if (iTotalIn == 6)
        return true;

    return true;*/
}

bool k3dFrustum::VolumeInFrustum(const kMath::Vec3f *vCorner)
{
    //int iTotalIn = 0;

    for (int p = 0; p < 6; ++p)
    {
        int iInCount = 8;
        int iPtIn = 1;

        for (int i = 0; i < 8; ++i)
        {
            if (m_Frustum[p][A] * (vCorner[i].x) + m_Frustum[p][B] * (vCorner[i].y) +
                m_Frustum[p][C] * (vCorner[i].z) + m_Frustum[p][D] < 0)
            {
                //iPtIn = 0;
                --iInCount;
            }
        }

        if (iInCount == 0)
            return false;

        //iTotalIn += iPtIn;
    }

    return true;

    /*if (iTotalIn == 6)
        return true;

    return true;*/
}

void k3dFrustum::GenerateVisibleObjects()
{
    visibleObjects.clear();
    visibleNotEmptyNodes.clear();

    main_scene->Octree.GetObjectsFrustum(this, visibleObjects, visibleNotEmptyNodes);

    /*for (int i=0; i<main_scene->mesh.ile_ob; i++)
    {
        CObiekt *ob = main_scene->mesh.Obiekty[i];
        visibleObjects.push_back(ob);
    }*/

    //con.Print("Objects: %d", visibleObjects.size());
}

bool k3dFrustum::LightAffectsVisibleObjects(k3dLight *sw)
{
    for (int i=0; i<visibleNotEmptyNodes.size(); i++)
    {
        k3dOctreeNode *node = visibleNotEmptyNodes[i];

        // loose
        if (sw->NodeInVolume(node->center, node->length))
        {
            //con.Print("true %d of %d", i, visibleNotEmptyNodes.size());
            return true;
        }
    }

    //con.Print("false %d", visibleNotEmptyNodes.size());
    return false;
}
