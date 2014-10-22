#include "k3dOctree.h"

#pragma warning (disable:4018)		// '<' : signed/unsigned mismatch
#pragma warning (disable:4244)		// truncation from 'double' to 'float'
#pragma warning (disable:4305)		// truncation from 'const double' to 'const float'
#pragma warning (disable:4390)		// empty controlled statement found; is this the intent?
#pragma warning (disable:4800)		// 'int' : forcing value to bool 'true' or 'false' (performance warning)

void k3dOctree::MakeOctree(k3dSceneManager *scene, int maxObjectsPerNode)
{
    this->scene = scene;
    this->maxObjectsPerNode = maxObjectsPerNode;

    root = new k3dOctreeNode();

    minBound = kMath::Vec3f(10000000000, 10000000000, 10000000000);
    maxBound = kMath::Vec3f(-10000000000, -10000000000, -10000000000);

    for (int i=0; i<scene->mesh.ile_ob; i++)
    {
        kSObject *ob = scene->mesh.Obiekty[i];
        //if (ob->animated_pos || ob->animated_rot)continue;
        //if (ob->skinned)continue;
        if (ob->minx < minBound.x)minBound.x = ob->minx;
        if (ob->miny < minBound.y)minBound.y = ob->miny;
        if (ob->minz < minBound.z)minBound.z = ob->minz;
        if (ob->maxx > maxBound.x)maxBound.x = ob->maxx;
        if (ob->maxy > maxBound.y)maxBound.y = ob->maxy;
        if (ob->maxz > maxBound.z)maxBound.z = ob->maxz;
    }

    len = max(maxBound.x - minBound.x, maxBound.y - minBound.y);
    len = max(len, maxBound.z - minBound.z);

    root->parent = NULL;
    root->center = (minBound+maxBound)/2;
    root->length = len;

    for (int i=0; i<scene->mesh.ile_ob; i++)
    {
        kSObject *ob = scene->mesh.Obiekty[i];

        //if (ob->animated_pos || ob->animated_rot)continue;
        //if (ob->skinned)continue;

        AddObject(ob);
    }

    /*// lights

    for (int i=0; i<scene->swiatla.size(); i++)
    {
        k3dLight *sw = &scene->swiatla[i];

        AddLight(sw);
    }*/
}

int ClassifyPlane(kMath::Vec3f n, float d, kSObject *ob)
{
    int plus = 0;
    int minus = 0;

    for (int i=0; i<8; i++)
    {
        if (n % ob->anim_BBoxCorners[i] + d >= 0)plus++;
        else minus++;
    }

    if (plus == 8)return 1;
    if (minus == 8)return 0;
    return 2;
}

bool ObjectInBox(kSObject *ob, kMath::Vec3f BoxMin, kMath::Vec3f BoxMax)
{
    for (int i=0; i<8; i++)
    {
        if (ob->anim_BBoxCorners[i].x < BoxMin.x)return false;
        if (ob->anim_BBoxCorners[i].y < BoxMin.y)return false;
        if (ob->anim_BBoxCorners[i].z < BoxMin.z)return false;
        if (ob->anim_BBoxCorners[i].x > BoxMax.x)return false;
        if (ob->anim_BBoxCorners[i].y > BoxMax.y)return false;
        if (ob->anim_BBoxCorners[i].z > BoxMax.z)return false;
    }

    return true;
}

int deep = 0;

void k3dOctree::AddToNode(k3dOctreeNode *node, kSObject *ob)
{
    bool splitable = false;
    int x, y, z;
    float dist = 99999999999;

    for (int i=0; i<2; i++)for (int o=0; o<2; o++)for (int p=0; p<2; p++)
    {
        kMath::Vec3f center = node->center;
        if (i == 0)center -= kMath::Vec3f(node->length/4, 0, 0);
        else center += kMath::Vec3f(node->length/4, 0, 0);
        if (o == 0)center -= kMath::Vec3f(0, node->length/4, 0);
        else center += kMath::Vec3f(0, node->length/4, 0);
        if (p == 0)center -= kMath::Vec3f(0, 0, node->length/4);
        else center += kMath::Vec3f(0, 0, node->length/4);

        if (ObjectInBox(ob, center - node->length/2, center + node->length/2))
        {
            float d = (ob->anim_Center - center).Length();
            if (d < dist)
            {
                splitable = true;
                dist = d;
                x = i;
                y = o;
                z = p;
            }
        }
    }

    if (node->children[0][0][0] == 0)
    {
        if (!splitable)
        {
            ob->deep = deep;
            ob->octreeNode = node;
            node->objects.push_back(ob);
        }
        else
        {
            ob->deep = deep;
            ob->octreeNode = node;
            node->splitableObjects.push_back(ob);
        }

        if ((node->splitableObjects.size()+node->objects.size()) >= maxObjectsPerNode &&
            node->splitableObjects.size() > 0)
        {
            // split node
            node->MakeChildren();

            std::vector<kSObject*> objects = node->splitableObjects;

            node->splitableObjects.clear();

            int last_deep = deep;
            for (int i=0; i<objects.size(); i++)
            {
                deep = last_deep;
                AddToNode(node, objects[i]);
            }
            deep = last_deep;
        }
    }
    else
    {
        if (splitable)
        {
            deep++;
            AddToNode(node->children[x][y][z], ob);
        }
        else
        {
            ob->deep = deep;
            ob->octreeNode = node;
            node->objects.push_back(ob);
        }

        // classic
//        int xc = ClassifyPlane(kMath::Vec3f(1, 0, 0), -node->center.x, ob);
//        int yc = ClassifyPlane(kMath::Vec3f(0, 1, 0), -node->center.y, ob);
//        int zc = ClassifyPlane(kMath::Vec3f(0, 0, 1), -node->center.z, ob);

//        if (xc == 2 || yc == 2 || zc == 2)
//        {
//          ob->octreeNode = node;
//          node->objects.push_back(ob);
//        }
//        else
//        {
//          AddToNode(node->children[xc][yc][zc], ob);
//        }
    }
}

void k3dOctree::AddToNode(k3dOctreeNode *node, k3dLight *sw)
{
    for (int i=0; i<node->objects.size(); i++)
    {
        if (sw->ObjectInVolume(node->objects[i]))
        {
            node->lights.push_back(sw);
            break;
        }
    }

    if (node->children[0][0][0] != 0)
    {
        for (int i=0; i<2; i++)for (int o=0; o<2; o++)for (int p=0; p<2; p++)
        {
            AddToNode(node->children[i][o][p], sw);
        }
    }
}

void k3dOctree::AddObject(kSObject *ob)
{
    kMath::Vec3f BoxMin = root->center - root->length;
    kMath::Vec3f BoxMax = root->center + root->length;
    while (!ObjectInBox(ob, BoxMin, BoxMax))
    {
        int x=-1, y=-1, z=-1;
        // must resize whole octree
        for (int i=0; i<8; i++)
        {
            if (ob->anim_BBoxCorners[i].x < BoxMin.x)x = 1;
            if (ob->anim_BBoxCorners[i].y < BoxMin.y)y = 1;
            if (ob->anim_BBoxCorners[i].z < BoxMin.z)z = 1;
            if (ob->anim_BBoxCorners[i].x > BoxMax.x)x = 0;
            if (ob->anim_BBoxCorners[i].y > BoxMax.y)y = 0;
            if (ob->anim_BBoxCorners[i].z > BoxMax.z)z = 0;
        }

        if (x == -1)x = 0;
        if (y == -1)y = 0;
        if (z == -1)z = 0;

        k3dOctreeNode *newRoot = new k3dOctreeNode();
        newRoot->length = root->length * 2;
        newRoot->parent = NULL;

        newRoot->center = root->center;

        if (x == 0)newRoot->center.x += root->length/2;
        else newRoot->center.x -= root->length/2;
        if (y == 0)newRoot->center.y += root->length/2;
        else newRoot->center.y -= root->length/2;
        if (z == 0)newRoot->center.z += root->length/2;
        else newRoot->center.z -= root->length/2;

        newRoot->MakeChildren();
        delete newRoot->children[x][y][z];
        newRoot->children[x][y][z] = root;

        root->parent = newRoot;
        root = newRoot;

        BoxMin = root->center - root->length;
        BoxMax = root->center + root->length;
    }

    deep = 0;
    AddToNode(root, ob);
}

void k3dOctree::AddLight(k3dLight *sw)
{
    deep = 0;
    AddToNode(root, sw);
}

void k3dOctree::DeleteObject(kSObject *ob)
{
    k3dOctreeNode *node = ob->octreeNode;

    for (int i=0; i<node->objects.size(); i++)
    {
        if (node->objects[i] == ob)
        {
            node->objects.erase(node->objects.begin() + i);

            if (node != root)JoinNodes(node->parent);

            return;
        }
    }

    for (int i=0; i<node->splitableObjects.size(); i++)
    {
        if (node->splitableObjects[i] == ob)
        {
            node->splitableObjects.erase(node->objects.begin() + i);

            if (node != root)JoinNodes(node->parent);

            return;
        }
    }
}

// joins node with its children if it is possible
void k3dOctree::JoinNodes(k3dOctreeNode *node)
{
    int obs = 0;

    for (int i=0; i<2; i++)for (int o=0; o<2; o++)for (int p=0; p<2; p++)
    {
        if (node->children[i][o][p]->children[0][0][0] != 0)
        {
            return;
        }
        obs += node->children[i][o][p]->objects.size();
        obs += node->children[i][o][p]->splitableObjects.size();
    }

    if (obs == 0)
    {
        for (int i=0; i<2; i++)for (int o=0; o<2; o++)for (int p=0; p<2; p++)
        {
            delete node->children[i][o][p];
            node->children[i][o][p] = 0;
        }
    }

    obs += node->objects.size();
    obs += node->splitableObjects.size();

    if (obs < maxObjectsPerNode)
    {
        if (node->children[0][0][0])for (int i=0; i<2; i++)for (int o=0; o<2; o++)for (int p=0; p<2; p++)
        {
            for (int u=0; u<node->children[i][o][p]->objects.size(); u++)
            {
                node->children[i][o][p]->objects[u]->octreeNode = node;
                node->splitableObjects.push_back(node->children[i][o][p]->objects[u]);
            }
            for (int u=0; u<node->children[i][o][p]->splitableObjects.size(); u++)
            {
                node->children[i][o][p]->objects[u]->octreeNode = node;
                node->splitableObjects.push_back(node->children[i][o][p]->objects[u]);
            }

            delete node->children[i][o][p];
            node->children[i][o][p] = 0;
        }

        if (node != root)JoinNodes(node->parent);
    }
}

void k3dOctree::UpdateObject(kSObject *ob)
{
    k3dOctreeNode *node = ob->octreeNode;

    // dalej sie miesci
    if (ObjectInBox(ob, node->center - node->length, node->center + node->length))
    {
        if (node->children[0][0][0])
        {
            for (int i=0; i<node->objects.size(); i++)
            {
                if (node->objects[i] == ob)
                {
                    node->objects.erase(node->objects.begin() + i);
                    AddToNode(node, ob);
                    return;
                }
            }

            for (int i=0; i<node->splitableObjects.size(); i++)
            {
                if (node->splitableObjects[i] == ob)
                {
                    node->splitableObjects.erase(node->objects.begin() + i);
                    AddToNode(node, ob);
                    return;
                }
            }
        }
    }
    else
    {
        DeleteObject(ob);
        AddObject(ob);
    }
}

void k3dOctreeNode::MakeChildren()
{
    for (int i=0; i<2; i++)for (int o=0; o<2; o++)for (int p=0; p<2; p++)
    {
        children[i][o][p] = new k3dOctreeNode();
        children[i][o][p]->length = length / 2;
        children[i][o][p]->center = center;
        children[i][o][p]->parent = this;
        if (i == 0)children[i][o][p]->center -= kMath::Vec3f(children[i][o][p]->length/2, 0, 0);
        else children[i][o][p]->center += kMath::Vec3f(children[i][o][p]->length/2, 0, 0);
        if (o == 0)children[i][o][p]->center -= kMath::Vec3f(0, children[i][o][p]->length/2, 0);
        else children[i][o][p]->center += kMath::Vec3f(0, children[i][o][p]->length/2, 0);
        if (p == 0)children[i][o][p]->center -= kMath::Vec3f(0, 0, children[i][o][p]->length/2);
        else children[i][o][p]->center += kMath::Vec3f(0, 0, children[i][o][p]->length/2);
    }
}

int zapytanO = 0;

void k3dOctree::AddObjectsFromNodeFrustum(std::vector<kSObject*> &obs, vector<k3dOctreeNode*> &visibleNotEmptyNodes,
                                        k3dOctreeNode *node, k3dFrustum *frustum, bool fullyInside)
{
    int inside = 0;

    if (!fullyInside)
    {
        zapytanO++;
        inside = frustum->SphereInFrustum(node->center.x, node->center.y, node->center.z, node->length*sqrt(3.0));

        if (inside == 2)fullyInside = true;
    }

    if (fullyInside || inside > 0)
    {
        if (node->objects.size() > 0 || node->splitableObjects.size())visibleNotEmptyNodes.push_back(node);

        obs.insert(obs.end(), node->objects.begin(), node->objects.end());
        obs.insert(obs.end(), node->splitableObjects.begin(), node->splitableObjects.end());

        if (node->children[0][0][0])
        {
            for (int i=0; i<2; i++)for (int o=0; o<2; o++)for (int p=0; p<2; p++)
                AddObjectsFromNodeFrustum(obs, visibleNotEmptyNodes, node->children[i][o][p], frustum, fullyInside);
        }
    }
}

int zapytanL = 0;

void k3dOctree::AddObjectsFromNodeLight(std::vector<kSObject*> &obs, vector<k3dOctreeNode*> &visibleNotEmptyNodes,
                                        k3dOctreeNode *node, k3dLight *sw, bool fullyInside)
{
    int inside = 0;

    if (!fullyInside)
    {
        zapytanL++;
        inside = sw->NodeInVolume(node->center, node->length);

        if (inside == 2)fullyInside = true;
    }

    if (fullyInside || inside > 0)
    {
        if (node->objects.size() > 0 || node->splitableObjects.size())visibleNotEmptyNodes.push_back(node);

        obs.insert(obs.end(), node->objects.begin(), node->objects.end());
        obs.insert(obs.end(), node->splitableObjects.begin(), node->splitableObjects.end());

        if (node->children[0][0][0])
        {
            for (int i=0; i<2; i++)for (int o=0; o<2; o++)for (int p=0; p<2; p++)
                AddObjectsFromNodeLight(obs, visibleNotEmptyNodes, node->children[i][o][p], sw, fullyInside);
        }
    }
}

void k3dOctree::GetObjectsFrustum(k3dFrustum *frustum, std::vector<kSObject*> &visibleObjects, vector<k3dOctreeNode*> &visibleNotEmptyNodes)
{
    zapytanO = 0;
    if (use_octree)
        AddObjectsFromNodeFrustum(visibleObjects, visibleNotEmptyNodes, main_scene->Octree.root, frustum, false);
    else
    {
        for (int i=0; i<main_scene->mesh.ile_ob; i++)
            visibleObjects.push_back(main_scene->mesh.Obiekty[i]);
    }
    //con.Print("ZapytanO %d", zapytanO);
}

void k3dOctree::GetObjectsLight(k3dLight *sw, std::vector<kSObject*> &visibleObjects, vector<k3dOctreeNode*> &visibleNotEmptyNodes)
{
    zapytanL = 0;
    if (use_octree)
        AddObjectsFromNodeLight(visibleObjects, visibleNotEmptyNodes, main_scene->Octree.root, sw, false);
    else
    {
        for (int i=0; i<main_scene->mesh.ile_ob; i++)
            visibleObjects.push_back(main_scene->mesh.Obiekty[i]);
    }
    //con.Print("ZapytanL %d", zapytanL);
}
